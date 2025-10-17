#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <optional>
#include "../ai-player/Collision.h"
#include "Enemy.h"

// ────────────────────────────────────────────────
// Path node for A*
// ────────────────────────────────────────────────
struct PathNode {
    glm::ivec2 cell;
    float gCost = 0.0f;
    float hCost = 0.0f;
    glm::ivec2 parent;
    float fCost() const { return gCost + hCost; }
};

// ✅ Hash function usable across Pathfinder + AI
struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const noexcept {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

// ────────────────────────────────────────────────
// Simple half-tile grid A*
// ────────────────────────────────────────────────
class Pathfinder {
public:
    Pathfinder(int tileW, int tileH) : tW(tileW), tH(tileH) {}

    std::vector<glm::vec2> FindPath(const glm::vec2& start, const glm::vec2& goal,
        const std::vector<const std::vector<std::vector<int>>*>& maps,
        const std::unordered_set<int>& solid, glm::vec2 mapSize,
        float clearanceRadius)
    {
        if (maps.empty()) return {};
        const float cellX = float(tW) * 0.5f, cellY = float(tH) * 0.5f;

        glm::ivec2 s = ToGrid(start, cellX, cellY);
        glm::ivec2 g = ToGrid(goal, cellX, cellY);
        if (s == g) return { goal };

        auto H = [&](glm::ivec2 a, glm::ivec2 b) {
            return glm::length(glm::vec2(a - b));
        };

        struct PairCmp {
            bool operator()(const std::pair<float, glm::ivec2>& a,
                            const std::pair<float, glm::ivec2>& b) const {
                return a.first > b.first;
            }
        };

        std::priority_queue<
            std::pair<float, glm::ivec2>,
            std::vector<std::pair<float, glm::ivec2>>,
            PairCmp> open;

        std::unordered_map<glm::ivec2, PathNode, IVec2Hash> nodes;
        std::unordered_set<glm::ivec2, IVec2Hash> closed;

        PathNode n{ s, 0.0f, H(s, g), s };
        nodes[s] = n;
        open.emplace(n.fCost(), s);

        const glm::ivec2 dirs[8] = {
            {1,0},{-1,0},{0,1},{0,-1},
            {1,1},{-1,1},{1,-1},{-1,-1}
        };

        while (!open.empty()) {
            glm::ivec2 cur = open.top().second; open.pop();
            if (closed.count(cur)) continue;
            closed.insert(cur);
            if (cur == g) return Reconstruct(nodes, cur, cellX, cellY);

            for (auto& d : dirs) {
                glm::ivec2 nxt = cur + d;
                // 🔒 Clamp to map grid bounds
                if (nxt.x < 0 || nxt.y < 0) continue;
                if (nxt.x >= int(mapSize.x / tW) || nxt.y >= int(mapSize.y / tH)) continue;
                if (closed.count(nxt)) continue;
                glm::vec2 world = ToWorld(nxt, cellX, cellY);

                // Check traversability: small probe circle must not hit solid
                Circle probe{ world, clearanceRadius };
                if (IsCircleBlocked(probe, maps, tW, tH, solid)) continue;

                float gNew = nodes[cur].gCost + glm::length(glm::vec2(d));
                if (!nodes.count(nxt) || gNew < nodes[nxt].gCost) {
                    PathNode np{ nxt, gNew, H(nxt,g), cur };
                    nodes[nxt] = np;
                    open.emplace(np.fCost(), nxt);
                }
            }
        }
        return {}; // no route
    }

private:
    int tW, tH;

    glm::ivec2 ToGrid(glm::vec2 p, float cx, float cy) const {
        return { int(p.x / cx), int(p.y / cy) };
    }
    glm::vec2 ToWorld(glm::ivec2 c, float cx, float cy) const {
        return { c.x * cx + cx * 0.5f, c.y * cy + cy * 0.5f };
    }

    std::vector<glm::vec2> Reconstruct(
        const std::unordered_map<glm::ivec2, PathNode, IVec2Hash>& nodes,
        glm::ivec2 goal, float cx, float cy)
    {
        std::vector<glm::vec2> out;
        glm::ivec2 cur = goal;
        while (nodes.count(cur) && nodes.at(cur).parent != cur) {
            out.push_back(ToWorld(cur, cx, cy));
            cur = nodes.at(cur).parent;
        }
        std::reverse(out.begin(), out.end());
        return out;
    }
};

// ────────────────────────────────────────────────
// EnemyAI – chase (LOS + view distance) & patrol
// ────────────────────────────────────────────────
class EnemyAI {
public:
    EnemyAI() = default;

    void Update(Enemy* e, const Circle& player,
        const std::vector<const std::vector<std::vector<int>>*>& maps,
        const std::unordered_set<int>& solid,
        int tW, int tH, float dt)
    {
        if (!e || maps.empty() || !maps[0] || maps[0]->empty()) return;

        Circle self = e->ComputeBoundingCircle();
        float dist = glm::distance(self.center, player.center);

        // Optional FOV: if you want a cone instead of 360°, set fovCos_ to cos(FOV/2).
        // We infer facing from last non-zero velocity; if none, use 360°.
        bool inFov = true;
        if (fovCos_ > -1.0f) {
            glm::vec2 dirTo = glm::normalize(player.center - self.center);
            float dp = glm::dot(dirFacing_, dirTo);
            inFov = (dp >= fovCos_);
        }

        bool hasLos = HasLineOfSight(self.center, player.center, maps, solid, tW, tH);
        bool see = (dist < vision_ && inFov && hasLos);

        if (state == Idle) {
            // Random patrol steps
            patrolT -= dt;
            if (see) {
                state = Chase;
                lastSeen_ = player.center;
                path.clear(); // force a fresh path below
                pathT = 0.0f;
            } else if (patrolT <= 0.0f) {
                patrolT = patrolInterval_;
                Patrol(e, maps, solid, tW, tH);
            }
        }

        if (state == Chase) {
            if (see) lastSeen_ = player.center;

            // If we lost sight for some time, give up and go Idle
            if (!see) {
                loseT += dt;
                if (loseT > loseTime_) {
                    state = Idle;
                    loseT = 0.0f;
                    path.clear();
                    return;
                }
            } else {
                loseT = 0.0f;
            }

            // Recompute path occasionally or when path is empty
            pathT -= dt;
            if (pathT <= 0.0f || path.empty()) {
                Pathfinder pf(tW, tH);
                glm::vec2 target = lastSeen_.has_value() ? *lastSeen_ : player.center;
                float clearance = self.radius * 0.90f; // a little margin
                path = pf.FindPath(self.center, target, maps, solid,
                                   { float((*maps[0])[0].size() * tW), float(maps[0]->size() * tH) },
                                   clearance);
                pathT = repathInterval_;
            }

            MoveAlong(e, maps, solid, tW, tH, dt);
        }

        // Update "facing" vector (for FOV) from actual movement
        // Use Enemy::GetPosition delta (top-left), convert to center delta
        // Approximate: use last 2 centers if available
        {
            glm::vec2 cur = e->ComputeBoundingCircle().center;
            glm::vec2 v = cur - lastCenter_;
            if (glm::length(v) > 0.01f) {
                dirFacing_ = glm::normalize(v);
            }
            lastCenter_ = cur;
        }
    }

    // Optional knobs
    void SetVision(float range)        { vision_ = range; }
    void SetFovDegrees(float degrees)  { fovCos_ = (degrees >= 359.0f) ? -1.0f : std::cos(glm::radians(degrees * 0.5f)); }
    void SetSpeed(float s)             { speed_ = s; }
    void SetRepathInterval(float sec)  { repathInterval_ = glm::clamp(sec, 0.1f, 2.0f); }
    void SetLoseTime(float sec)        { loseTime_ = glm::clamp(sec, 0.2f, 5.0f); }

private:
    enum State { Idle, Chase };
    State state = Idle;

    float stuckT_ = 0.0f;
    float lastProgressDist_ = std::numeric_limits<float>::infinity();

    // Pathing / memory
    std::vector<glm::vec2> path;
    std::optional<glm::vec2> lastSeen_;
    float pathT = 0.0f;

    // Sensing
    float vision_ = 300.0f;
    float fovCos_ = -1.0f;           // -1 = 360°; set via SetFovDegrees()
    glm::vec2 dirFacing_ = {1, 0};   // forward for FOV
    float loseTime_ = 2.0f;          // seconds to forget target once out of sight
    float loseT = 0.0f;

    // Patrol
    float patrolT = 0.0f;
    float patrolInterval_ = 2.5f;
    const float step_ = 16.0f;

    // Motion
    float speed_ = 60.0f;
    float repathInterval_ = 0.75f;

    // bookkeeping
    glm::vec2 lastCenter_{0.0f};

    // ── Sightline check: sample along the segment and fail if any solid tile blocks ──
    static bool HasLineOfSight(const glm::vec2& a, const glm::vec2& b,
                               const std::vector<const std::vector<std::vector<int>>*>& maps,
                               const std::unordered_set<int>& solid,
                               int tW, int tH)
    {
        // Sample every ~quarter tile along the ray with a tiny probe circle
        glm::vec2 d = b - a;
        float len = glm::length(d);
        if (len < 1e-4f) return true;
        glm::vec2 dir = d / len;

        const float step = 0.25f * std::min(tW, tH);
        const float probeR = 0.2f * std::min(tW, tH);
        int samples = std::max(1, int(len / step));

        for (int i = 1; i <= samples; ++i) {
            glm::vec2 p = a + dir * (i * step);
            Circle probe{ p, probeR };
            if (IsCircleBlocked(probe, maps, tW, tH, solid)) {
                return false;
            }
        }
        return true;
    }

    void Patrol(Enemy* e,
        const std::vector<const std::vector<std::vector<int>>*>& maps,
        const std::unordered_set<int>& solid,
        int tW, int tH)
    {
        static glm::vec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
        Circle c = e->ComputeBoundingCircle();

        // Try up to 4 random directions to step without clipping
        for (int tries = 0; tries < 4; ++tries) {
            glm::vec2 d = dirs[rand() % 4];
            Circle n = c; n.center += d * step_;
            if (!IsCircleBlocked(n, maps, tW, tH, solid)) {
                e->SetCenter(n.center);
                break;
            }
        }
    }

    void MoveAlong(Enemy *e,
                   const std::vector<const std::vector<std::vector<int>>*> &maps,
                   const std::unordered_set<int> &solid,
                   int tW, int tH, float dt) {
        if (path.empty()) return;

        // --- Current & target ---
        Circle cur = e->ComputeBoundingCircle();
        glm::vec2 tgt = path.front();

        // Consider node reached if we’re close enough (prevents orbiting the point).
        const float reachRadius = 0.35f * std::min(tW, tH);
        glm::vec2 delta = tgt - cur.center;
        float dist = glm::length(delta);
        if (dist <= reachRadius) {
            path.erase(path.begin());
            lastProgressDist_ = std::numeric_limits<float>::infinity();
            stuckT_ = 0.0f;
            return;
        }

        glm::vec2 dir = delta / dist;
        glm::vec2 vel = dir * speed_;

        // IMPORTANT: let TryMoveCircle do the actual motion from *current* position.
        // No pre-move here (removes the double-move bug).
        glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);
        Circle test = cur;

        if (TryMoveCircle(test, vel, dt, mapSize, maps, solid, tW, tH)) {
            e->SetCenter(test.center);
        } else {
            // Collision blocked our direct move. Try a tiny perpendicular nudge to slide the corner.
            glm::vec2 perp = glm::normalize(glm::vec2(-dir.y, dir.x));
            glm::vec2 sideVel = perp * (0.6f * speed_);
            Circle side = cur;
            // Try both sides
            if (TryMoveCircle(side, sideVel, dt * 0.33f, mapSize, maps, solid, tW, tH)) {
                e->SetCenter(side.center);
            } else {
                side = cur;
                if (TryMoveCircle(side, -sideVel, dt * 0.33f, mapSize, maps, solid, tW, tH)) {
                    e->SetCenter(side.center);
                } else {
                    // Couldn’t slide; drop this node so we don’t headbutt it forever.
                    path.erase(path.begin());
                }
            }
        }

        // --- progress tracking: if we’re not closing distance, repath or skip node ---
        float nowDist = glm::length(tgt - e->ComputeBoundingCircle().center);
        if (nowDist > lastProgressDist_ - 0.25f) {          // not noticeably better
            stuckT_ += dt;
            if (stuckT_ > 0.4f) {                           // stuck ~0.4s
                // Drop this node; if path becomes empty, Update() will repath soon
                if (!path.empty()) path.erase(path.begin());
                stuckT_ = 0.0f;
                lastProgressDist_ = std::numeric_limits<float>::infinity();
                return;
            }
        } else {
            stuckT_ = 0.0f;
            lastProgressDist_ = nowDist;
        }
    }
};


#endif // ENEMY_AI_H

#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdlib> // rand
#include "../ai-player/Collision.h"
#include "Enemy.h"

// ────────────────────────────────────────────────
// Integer grid node + hash
// ────────────────────────────────────────────────
struct PathNode {
    glm::ivec2 cell{};
    float gCost = 0.0f;
    float hCost = 0.0f;
    glm::ivec2 parent{};
    float fCost() const noexcept { return gCost + hCost; }
};

struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const noexcept {
        return (std::hash<int>()(v.x) << 1) ^ std::hash<int>()(v.y);
    }
};

// ────────────────────────────────────────────────
// Pathfinder: half-tile A* with robust bounds, corner rules,
// start/goal snapping, and soft clearance near walls.
// ────────────────────────────────────────────────
class Pathfinder {
public:
    Pathfinder(int tileW, int tileH) : tW(tileW), tH(tileH) {}

    std::vector<glm::vec2> FindPath(const glm::vec2& start,
                                    const glm::vec2& goal,
                                    const std::vector<const std::vector<std::vector<int>>*>& maps,
                                    const std::unordered_set<int>& solidSet,
                                    glm::vec2 mapSize,
                                    float actorRadius) {
        if (maps.empty()) return {};

        const float cx = tW * 0.5f, cy = tH * 0.5f;
        const int maxX = std::max(1, int(mapSize.x / tW));
        const int maxY = std::max(1, int(mapSize.y / tH));
        const float probeR = std::max(1.0f, actorRadius * 0.6f); // softened so wall-hugging works

        auto toGrid  = [&](glm::vec2 p) { return glm::ivec2(int(p.x / cx), int(p.y / cy)); };
        auto toWorld = [&](glm::ivec2 c) { return glm::vec2(c.x * cx + cx * 0.5f, c.y * cy + cy * 0.5f); };

        auto walkable = [&](glm::ivec2 c) {
            if (c.x < 0 || c.y < 0 || c.x >= maxX || c.y >= maxY) return false;
            Circle probe{ toWorld(c), probeR };
            return !IsCircleBlocked(probe, maps, tW, tH, solidSet);
        };

        auto snapNearestFree = [&](glm::ivec2 s) {
            if (walkable(s)) return s;
            static const glm::ivec2 ring[8] = {
                {1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,1},{1,-1},{-1,-1}
            };
            for (int r=1; r<=2; ++r) {
                for (auto& d : ring) {
                    glm::ivec2 n = s + d * r;
                    if (n.x < 0 || n.y < 0 || n.x >= maxX || n.y >= maxY) continue;
                    if (walkable(n)) return n;
                }
            }
            return s; // give up; let A* handle it
        };

        glm::ivec2 s = snapNearestFree(toGrid(start));
        glm::ivec2 g = snapNearestFree(toGrid(goal));
        if (s == g) return { goal };

        auto H = [&](glm::ivec2 a, glm::ivec2 b) {
            // octile distance (better tie-breaking for 8-connected grids)
            glm::ivec2 d = b - a;
            float dx = std::abs((float)d.x), dy = std::abs((float)d.y);
            return (dx + dy) + (1.41421356f - 2.0f) * std::min(dx, dy);
        };

        struct PairCmp {
            bool operator()(const std::pair<float, glm::ivec2>& a,
                            const std::pair<float, glm::ivec2>& b) const noexcept {
                return a.first > b.first; // min-heap by fCost
            }
        };

        std::priority_queue<
            std::pair<float, glm::ivec2>,
            std::vector<std::pair<float, glm::ivec2>>,
            PairCmp> open;

        std::unordered_map<glm::ivec2, PathNode, IVec2Hash> nodes;
        std::unordered_set<glm::ivec2, IVec2Hash> closed;

        nodes[s] = { s, 0.0f, H(s, g), s };
        open.emplace(nodes[s].fCost(), s);

        static const glm::ivec2 dirs[8] = {
            {1,0},{-1,0},{0,1},{0,-1},
            {1,1},{1,-1},{-1,1},{-1,-1}
        };

        // Safety cap to avoid runaway if map is malformed
        const int kMaxExpansions = std::max(1024, maxX * maxY * 4);
        int expansions = 0;

        while (!open.empty()) {
            glm::ivec2 cur = open.top().second;
            open.pop();
            if (closed.count(cur)) continue;
            closed.insert(cur);
            if (cur == g) return Reconstruct(nodes, cur, toWorld);
            if (++expansions > kMaxExpansions) break;

            for (auto& d : dirs) {
                glm::ivec2 nxt = cur + d;
                if (nxt.x < 0 || nxt.y < 0 || nxt.x >= maxX || nxt.y >= maxY) continue;
                if (closed.count(nxt)) continue;

                // diagonal corner prevention (don’t cut through solid corners)
                if (d.x != 0 && d.y != 0) {
                    if (!walkable({cur.x + d.x, cur.y}) && !walkable({cur.x, cur.y + d.y}))
                        continue;
                }

                if (!walkable(nxt)) continue;

                float gNew = nodes[cur].gCost + ((d.x == 0 || d.y == 0) ? 1.0f : 1.41421356f);
                auto it = nodes.find(nxt);
                if (it == nodes.end() || gNew < it->second.gCost) {
                    nodes[nxt] = { nxt, gNew, H(nxt, g), cur };
                    open.emplace(nodes[nxt].fCost(), nxt);
                }
            }
        }
        return {}; // no route found
    }

private:
    int tW, tH;

    static std::vector<glm::vec2> Reconstruct(const std::unordered_map<glm::ivec2, PathNode, IVec2Hash>& nodes,
                                              glm::ivec2 goal,
                                              const std::function<glm::vec2(glm::ivec2)>& toWorld) {
        std::vector<glm::vec2> pts;
        glm::ivec2 cur = goal;
        while (nodes.count(cur) && nodes.at(cur).parent != cur) {
            pts.push_back(toWorld(cur));
            cur = nodes.at(cur).parent;
        }
        std::reverse(pts.begin(), pts.end());
        return pts;
    }
};

// ────────────────────────────────────────────────
// EnemyAI – deterministic chase with:
//  • LOS direct steering (no A* when visible)
//  • robust A* when not visible (last-seen target)
//  • stuck detection + quick repath
//  • stable waypoint consumption
//  • optional FOV, lose-time, and repath controls
// ────────────────────────────────────────────────
class EnemyAI {
public:
    EnemyAI() = default;

    // Tunables
    void SetVision(float v)            { vision_ = v; }
    void SetSpeed(float s)             { speed_ = s; }
    void SetLoseTime(float sec)        { loseTime_ = glm::clamp(sec, 0.1f, 5.0f); }
    void SetRepathInterval(float sec)  { repathInterval_ = glm::clamp(sec, 0.1f, 2.0f); }
    void SetFovDegrees(float deg)      { fovCos_ = (deg >= 359.0f) ? -1.0f : std::cos(glm::radians(deg * 0.5f)); }
    void SetAlertDistance(float px)    { alertDist_ = std::max(0.0f, px); } // “hearing/suspicion” start even without LOS

    void Update(Enemy* e, const Circle& player,
                const std::vector<const std::vector<std::vector<int>>*>& maps,
                const std::unordered_set<int>& solidSet,
                int tW, int tH, float dt)
    {
        if (!e || dt <= 0.0f || maps.empty() || !maps[0] || maps[0]->empty()) return;

        Circle self = e->ComputeBoundingCircle();
        const glm::vec2 toPlayer = player.center - self.center;
        const float dist = glm::length(toPlayer);

        // Perception: LOS + (optional) FOV + proximity alert
        bool inFov = true;
        if (fovCos_ > -1.0f && dist > 1e-4f) {
            glm::vec2 dirTo = toPlayer / dist;
            inFov = (glm::dot(dirFacing_, dirTo) >= fovCos_);
        }

        const bool los = HasLineOfSight(self.center, player.center, maps, solidSet, tW, tH);
        const bool see = (dist < vision_) && inFov && los;
        const bool proximityAlert = (dist < alertDist_);

        switch (state_) {
            case State::Idle:
                if (see || proximityAlert) {
                    StartChase(player.center);
                } else {
                    // Continuous idle roaming
                    UpdateIdleRoam(e, maps, solidSet, tW, tH, dt);
                }
                break;


            case State::Chase:
                // Maintain last known position
                if (see) lastSeen_ = player.center;

                // Give up only after sustained loss of contact
                if (!see) {
                    loseT_ += dt;
                    if (loseT_ > loseTime_) {
                        state_ = State::Idle;
                        loseT_ = 0.0f;
                        path_.clear();
                        break;
                    }
                } else {
                    loseT_ = 0.0f;
                }

                // If LOS right now, steer directly (no A*)
                if (see) {
                    FollowDirect(e, player.center, maps, solidSet, tW, tH, dt);
                    // quick bail if we’re moving; path will be recomputed lazily if needed
                    break;
                }

                // Otherwise follow/compute path to lastSeen_
                pathT_ -= dt;
                if (path_.empty() || pathT_ <= 0.0f) {
                    ComputePath(e, maps, solidSet, tW, tH, lastSeen_.value_or(player.center));
                }
                FollowPath(e, maps, solidSet, tW, tH, dt);
                break;
        }

        UpdateFacing(e);
    }

private:
    // State
    enum class State { Idle, Chase };
    State state_ = State::Idle;

    // Config
    float vision_         = 300.0f;
    float speed_          = 60.0f;
    float loseTime_       = 2.0f;    // how long to keep chasing after LOS broken
    float repathInterval_ = 0.5f;    // seconds between A* computations
    float alertDist_      = 64.0f;   // start chase even without LOS if very close
    float fovCos_         = -1.0f;   // -1 => 360°; else cos(FOV/2)

    // Timers
    float loseT_   = 0.0f;
    float pathT_   = 0.0f;
    float patrolT_ = 0.0f;

    // Patrol
    float patrolInterval_ = 2.5f;

    // Steering / memory
    glm::vec2 dirFacing_{1,0};
    glm::vec2 lastCenter_{0.0f};
    std::optional<glm::vec2> lastSeen_;
    std::vector<glm::vec2> path_;

    // Stuck tracking
    float stuckT_ = 0.0f;
    float lastProgressDist_ = std::numeric_limits<float>::infinity();

    void StartChase(glm::vec2 pos) {
        state_    = State::Chase;
        lastSeen_ = pos;
        path_.clear();
        pathT_    = 0.0f;
        stuckT_   = 0.0f;
        lastProgressDist_ = std::numeric_limits<float>::infinity();
    }

    static bool HasLineOfSight(const glm::vec2& a, const glm::vec2& b,
                               const std::vector<const std::vector<std::vector<int>>*>& maps,
                               const std::unordered_set<int>& solidSet,
                               int tW, int tH)
    {
        glm::vec2 d = b - a;
        float len = glm::length(d);
        if (len < 1e-4f) return true;
        glm::vec2 dir = d / len;

        const float step = 0.25f * std::min(tW, tH);
        const float r    = 0.20f * std::min(tW, tH);
        const int samples = std::max(1, int(len / step));

        for (int i = 1; i <= samples; ++i) {
            glm::vec2 p = a + dir * (i * step);
            Circle probe{ p, r };
            if (IsCircleBlocked(probe, maps, tW, tH, solidSet))
                return false;
        }
        return true;
    }

    // Direct “seek” while LOS is true
    void FollowDirect(Enemy* e,
                      const glm::vec2& target,
                      const std::vector<const std::vector<std::vector<int>>*>& maps,
                      const std::unordered_set<int>& solidSet,
                      int tW, int tH, float dt)
    {
        Circle cur = e->ComputeBoundingCircle();
        glm::vec2 delta = target - cur.center;
        float dist = glm::length(delta);
        if (dist < 1e-3f) return;

        glm::vec2 dir = delta / dist;
        glm::vec2 vel = dir * speed_;
        glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);

        Circle move = cur;
        if (TryMoveCircle(move, vel, dt, mapSize, maps, solidSet, tW, tH)) {
            e->SetCenter(move.center);
            return;
        }

        // Slide if blocked
        TrySlide(e, cur, dir, dt, maps, solidSet, tW, tH);
    }

    void ComputePath(Enemy* e,
                     const std::vector<const std::vector<std::vector<int>>*>& maps,
                     const std::unordered_set<int>& solidSet,
                     int tW, int tH, glm::vec2 target)
    {
        Circle self = e->ComputeBoundingCircle();
        Pathfinder pf(tW, tH);
        const glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);

        path_ = pf.FindPath(self.center, target, maps, solidSet, mapSize, self.radius);
        pathT_ = repathInterval_;

        // If no path returned, try a gentle nudge target (1/2 tile) to help around corners.
        if (path_.empty()) {
            static const glm::vec2 nudge[4] = {{8,0},{-8,0},{0,8},{0,-8}};
            for (auto &off : nudge) {
                path_ = pf.FindPath(self.center, target + off, maps, solidSet, mapSize, self.radius);
                if (!path_.empty()) break;
            }
        }
    }

    void FollowPath(Enemy* e,
                    const std::vector<const std::vector<std::vector<int>>*>& maps,
                    const std::unordered_set<int>& solidSet,
                    int tW, int tH, float dt)
    {
        if (path_.empty()) return;

        Circle cur = e->ComputeBoundingCircle();
        glm::vec2 tgt = path_.front();
        glm::vec2 delta = tgt - cur.center;
        float dist = glm::length(delta);

        const float reachRadius = 0.35f * std::min(tW, tH);
        if (dist <= reachRadius) {
            path_.erase(path_.begin());
            lastProgressDist_ = std::numeric_limits<float>::infinity();
            stuckT_ = 0.0f;
            return;
        }

        glm::vec2 dir = delta / std::max(dist, 1e-4f);
        glm::vec2 vel = dir * speed_;
        glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);

        Circle move = cur;
        bool moved = false;

        if (TryMoveCircle(move, vel, dt, mapSize, maps, solidSet, tW, tH)) {
            e->SetCenter(move.center);
            moved = true;
        } else {
            // Try slide both sides
            moved = TrySlide(e, cur, dir, dt, maps, solidSet, tW, tH);
            if (!moved) {
                // Drop this node and force fast repath next tick
                if (!path_.empty()) path_.erase(path_.begin());
                pathT_ = 0.0f;
                return;
            }
        }

        // Progress / stuck detection
        float nowDist = glm::length(tgt - e->ComputeBoundingCircle().center);
        if (nowDist > lastProgressDist_ - 0.25f) {
            stuckT_ += dt;
            if (stuckT_ > 0.35f) {
                // We’re not making real progress -> kick a repath soon
                pathT_ = 0.0f;
                stuckT_ = 0.0f;
                lastProgressDist_ = std::numeric_limits<float>::infinity();
            }
        } else {
            stuckT_ = 0.0f;
            lastProgressDist_ = nowDist;
        }
    }

    static bool TrySlide(Enemy* e, const Circle& cur, const glm::vec2& dir,
                         float dt,
                         const std::vector<const std::vector<std::vector<int>>*>& maps,
                         const std::unordered_set<int>& solidSet,
                         int tW, int tH)
    {
        float len = glm::length(dir);
        if (len < 1e-4f) return false;

        glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);
        glm::vec2 perp = glm::vec2(-dir.y, dir.x) / len;

        for (float side : {1.f, -1.f}) {
            Circle sideC = cur;
            glm::vec2 sideVel = perp * side * (0.6f * 60.0f);
            if (TryMoveCircle(sideC, sideVel, dt * 0.33f, mapSize, maps, solidSet, tW, tH)) {
                e->SetCenter(sideC.center);
                return true;
            }
        }
        return false;
    }

    void Patrol(Enemy* e,
                const std::vector<const std::vector<std::vector<int>>*>& maps,
                const std::unordered_set<int>& solidSet,
                int tW, int tH)
    {
        static const glm::vec2 dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};
        Circle c = e->ComputeBoundingCircle();
        for (int i=0; i<4; ++i) {
            glm::vec2 d = dirs[std::rand() % 4];
            Circle n = c; n.center += d * float(std::min(tW, tH));
            if (!IsCircleBlocked(n, maps, tW, tH, solidSet)) {
                e->SetCenter(n.center);
                break;
            }
        }
    }

    void UpdateFacing(Enemy* e) {
        glm::vec2 cur = e->ComputeBoundingCircle().center;
        glm::vec2 delta = cur - lastCenter_;
        float len = glm::length(delta);
        if (len > 0.01f) dirFacing_ = delta / len;
        lastCenter_ = cur;
    }


// ────────────────────────────────────────────────
// Smooth roaming when idle
// ────────────────────────────────────────────────

// Roaming state
bool roaming_ = false;
glm::vec2 roamTarget_{0.0f};
float roamTimer_ = 0.0f;
float roamInterval_ = 5.0f;  // seconds between picking new roam targets
float roamSpeed_ = 30.0f;    // slower than chase speed

void UpdateIdleRoam(Enemy* e,
                    const std::vector<const std::vector<std::vector<int>>*>& maps,
                    const std::unordered_set<int>& solidSet,
                    int tW, int tH, float dt)
{
    Circle cur = e->ComputeBoundingCircle();

    // Pick new target occasionally or if we reached the old one
    roamTimer_ -= dt;
    if (!roaming_ || roamTimer_ <= 0.0f ||
        glm::length(roamTarget_ - cur.center) < 4.0f) {

        roaming_ = true;
        roamTimer_ = roamInterval_ + (rand() % 3000) / 1000.0f; // randomize interval ±3s

        // Pick a random nearby direction and distance
        float angle = (rand() % 360) * 3.1415926f / 180.0f;
        float distance = 48.0f + float(rand() % 64); // 48–112px radius
        glm::vec2 candidate = cur.center + glm::vec2(std::cos(angle), std::sin(angle)) * distance;

        // Clamp within map and ensure not blocked
        glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);
        candidate.x = glm::clamp(candidate.x, 0.0f, mapSize.x - 1.0f);
        candidate.y = glm::clamp(candidate.y, 0.0f, mapSize.y - 1.0f);

        Circle probe{ candidate, cur.radius };
        if (IsCircleBlocked(probe, maps, tW, tH, solidSet)) {
            roaming_ = false; // skip if blocked
            return;
        }
        roamTarget_ = candidate;
    }

    // Move toward roam target smoothly
    glm::vec2 delta = roamTarget_ - cur.center;
    float dist = glm::length(delta);
    if (dist < 1.0f) return;

    glm::vec2 dir = delta / dist;
    glm::vec2 vel = dir * roamSpeed_;
    glm::vec2 mapSize(maps[0]->at(0).size() * tW, maps[0]->size() * tH);

    Circle move = cur;
    if (TryMoveCircle(move, vel, dt, mapSize, maps, solidSet, tW, tH))
        e->SetCenter(move.center);
    else
        TrySlide(e, cur, dir, dt, maps, solidSet, tW, tH);
}


};



#endif // ENEMY_AI_H

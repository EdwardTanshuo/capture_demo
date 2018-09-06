#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#define QUAD 4

#define VEC(coor_a, coor_b) (Coordinate{(coor_b).x-(coor_a).x,(coor_b).y-(coor_a).y})

#define NORM(coor) (std::sqrt((coor).x*(coor).x+(coor).y*(coor).y))

#define DIST(coor_a, coor_b) (std::sqrt(((coor_a).x-(coor_b).x)*((coor_a).x-(coor_b).x)+ \
((coor_a).y-(coor_b).y)*((coor_a).y-(coor_b).y)))

#define INNER_PRODUCT(coor_a, coor_b) ((coor_a).x*(coor_b).x+(coor_a).y*(coor_b).y)

#define PROJ(coor_a, coor_b) INNER_PRODUCT((coor_a),(coor_b))/NORM((coor_b))

#define COS_ANGLE(coor_a, coor_b) INNER_PRODUCT((coor_a),(coor_b))/NORM((coor_a))/NORM((coor_b))

namespace tree {
    struct TreeNode;

    struct Coordinate {
        float x;
        float y;
    };

    const Coordinate up_unit_vec{ 0.0f, -1.0f };
    const Coordinate down_unit_vec{ 0.0f, 1.0f };
    const Coordinate left_unit_vec{ -1.0f, 0.0f };
    const Coordinate right_unit_vec{ 1.0f, 0.0f };

    enum Direction {
        UP = 0,
        DOWN = 1,
        LEFT = 2,
        RIGHT = 3
    };

    struct TreeNode {
    private:
        //quad tree branches
        std::array<TreeNode*, QUAD> children;

    public:
        int x;
        int y;
        Coordinate coor;
        std::wstring barcode;

        TreeNode(int x, int y, std::wstring barcode) : barcode(barcode) {
            std::fill(this->children.begin(), this->children.end(), nullptr);
            coor.x = float(x);
            coor.y = float(y);
            this->x = 0;
            this->y = 0;

            // Add noise to avoid trival vector
            if (coor.x == 0 && coor.y == 0) {
                coor.x = 1;
                coor.y = 1;
            }
        };

        float norm() const {
            return NORM(this->coor);
        }

        std::array<TreeNode*, QUAD> get_children() const {
            return this->children;
        }

        TreeNode* get_child(Direction direction) const {
            return this->children[direction];
        }

        void insert_child(TreeNode* new_node, Direction direction) {
            this->children[direction] = new_node;
        }

        void insert_child(TreeNode* new_node, bool& flag, float max_cos_angle, float max_dist_h, float max_dist_v) {
            if (is_next_up(this, new_node, max_cos_angle, max_dist_v) && this->children[UP] == nullptr) {
                is_next_up(this, new_node, max_cos_angle, max_dist_v);
                this->children[UP] = new_node;
                flag = true;
            }
            else if (is_next_down(this, new_node, max_cos_angle, max_dist_v) && this->children[DOWN] == nullptr) {
                this->children[DOWN] = new_node;
                flag = true;
            }
            else if (is_next_left(this, new_node, max_cos_angle, max_dist_h) && this->children[LEFT] == nullptr) {
                this->children[LEFT] = new_node;
                flag = true;
            }
            else if (is_next_right(this, new_node, max_cos_angle, max_dist_h) && this->children[RIGHT] == nullptr) {
                this->children[RIGHT] = new_node;
                flag = true;
            }
            flag = false;
        }

        static float distance(const TreeNode* a, const TreeNode* b) {
            return DIST(a->coor, b->coor);
        }

        static float projection(const TreeNode* from, const TreeNode* to) {
            return PROJ(from->coor, to->coor);
        }

        static Coordinate gen_vec(const TreeNode* from, const TreeNode* to) {
            return VEC(from->coor, to->coor);
        }

        static float consine_angle(Coordinate a, Coordinate b) {
            return COS_ANGLE(a, b);
        }

        static bool is_next_up(const TreeNode* from, const TreeNode* to, float max_cos_angle, float max_dist) {
            return COS_ANGLE(VEC(from->coor, to->coor), up_unit_vec) >= max_cos_angle
                &&
                DIST(from->coor, to->coor) <= max_dist;
        }

        static bool is_next_down(const TreeNode* from, const TreeNode* to, float max_cos_angle, float max_dist) {
            return COS_ANGLE(VEC(from->coor, to->coor), down_unit_vec) >= max_cos_angle
                &&
                DIST(from->coor, to->coor) <= max_dist;
        }

        static bool is_next_left(const TreeNode* from, const TreeNode* to, float max_cos_angle, float max_dist) {
            return COS_ANGLE(VEC(from->coor, to->coor), left_unit_vec) >= max_cos_angle
                &&
                DIST(from->coor, to->coor) <= max_dist;
        }

        static bool is_next_right(const TreeNode* from, const TreeNode* to, float max_cos_angle, float max_dist) {
            return COS_ANGLE(VEC(from->coor, to->coor), right_unit_vec) >= max_cos_angle
                &&
                DIST(from->coor, to->coor) <= max_dist;
        }

        static bool is_up(const TreeNode* from, const TreeNode* to, float max_cos_angle) {
            return COS_ANGLE(VEC(from->coor, to->coor), up_unit_vec) >= max_cos_angle;
        }

        static bool is_down(const TreeNode* from, const TreeNode* to, float max_cos_angle) {
            return COS_ANGLE(VEC(from->coor, to->coor), down_unit_vec) >= max_cos_angle;
        }

        static bool is_left(const TreeNode* from, const TreeNode* to, float max_cos_angle) {
            return COS_ANGLE(VEC(from->coor, to->coor), left_unit_vec) >= max_cos_angle;
        }

        static bool is_right(const TreeNode* from, const TreeNode* to, float max_cos_angle) {
            return COS_ANGLE(VEC(from->coor, to->coor), right_unit_vec) >= max_cos_angle;
        }

        static std::vector<TreeNode*> sort_nodes(
            std::vector<TreeNode*> nodes,
            std::vector<std::pair<Coordinate, Coordinate>> &edges,
            float max_cos_angle,
            float max_dist_h,
            float max_dist_v,
            int w,
            int h
        ) throw(std::exception);

    };
}

#endif

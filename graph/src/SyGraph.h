#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <utility>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>
namespace SyLib
{
    // 图类（邻接表实现）
    template <typename T>
    class Graph
    {
    public:
        // 构造函数：默认创建无向图
        Graph(bool is_directed = false) : is_directed_(is_directed) {}

        // ========== 基础功能==========
        // 1. 添加顶点
        void add_vertex(const T &vertex)
        {
            if (adj_list_.find(vertex) == adj_list_.end())
            {
                adj_list_[vertex] = std::vector<std::pair<T, int>>();
            }
        }

        // 2. 添加边（支持权重，默认权重为1）
        void add_edge(const T &v1, const T &v2, int weight = 1)
        {
            // 确保两个顶点都存在
            add_vertex(v1);
            add_vertex(v2);

            // 检查边是否已存在（避免重复添加）
            if (has_edge(v1, v2))
            {
                // 适配任意类型顶点的错误提示：统一转成字符串
                auto to_string_helper = [](const T &val) -> std::string
                {
                    // 如果是字符串类型，直接返回
                    if constexpr (std::is_same_v<T, std::string>)
                    {
                        return val;
                    }
                    // 如果是数值类型，转成字符串
                    else if constexpr (std::is_arithmetic_v<T>)
                    {
                        return std::to_string(val);
                    }
                    // 其他类型（如自定义类型），返回默认标识
                    else
                    {
                        return "[unknown vertex]";
                    }
                };

                // 拼接错误信息
                std::string err_msg = "Edge already exists between ";
                err_msg += to_string_helper(v1);
                err_msg += " and ";
                err_msg += to_string_helper(v2);
                throw std::runtime_error(err_msg);
            }

            // 有向图：仅添加 v1->v2
            adj_list_[v1].emplace_back(v2, weight);
            // 无向图：额外添加 v2->v1
            if (!is_directed_)
            {
                adj_list_[v2].emplace_back(v1, weight);
            }
        }

        // 3. 移除边
        void remove_edge(const T &v1, const T &v2)
        {
            if (adj_list_.find(v1) == adj_list_.end() || adj_list_.find(v2) == adj_list_.end())
            {
                throw std::invalid_argument("Vertex does not exist");
            }

            // 移除 v1->v2 的边
            auto &neighbors = adj_list_[v1];
            for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
            {
                if (it->first == v2)
                {
                    neighbors.erase(it);
                    break;
                }
            }

            // 无向图：移除 v2->v1 的边
            if (!is_directed_)
            {
                auto &neighbors2 = adj_list_[v2];
                for (auto it = neighbors2.begin(); it != neighbors2.end(); ++it)
                {
                    if (it->first == v1)
                    {
                        neighbors2.erase(it);
                        break;
                    }
                }
            }
        }

        // 4. 移除顶点（同时移除所有关联的边）
        void remove_vertex(const T &vertex)
        {
            if (adj_list_.find(vertex) == adj_list_.end())
            {
                throw std::invalid_argument("Vertex does not exist");
            }

            // 第一步：移除所有指向该顶点的边
            for (auto &pair : adj_list_)
            {
                auto &neighbors = pair.second;
                for (auto it = neighbors.begin(); it != neighbors.end();)
                {
                    if (it->first == vertex)
                    {
                        it = neighbors.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            // 第二步：删除顶点本身
            adj_list_.erase(vertex);
        }

        // 5. 检查两个顶点是否有边连接
        bool has_edge(const T &v1, const T &v2) const
        {
            if (adj_list_.find(v1) == adj_list_.end())
            {
                return false;
            }

            const auto &neighbors = adj_list_.at(v1);
            for (const auto &pair : neighbors)
            {
                if (pair.first == v2)
                {
                    return true;
                }
            }
            return false;
        }

        // 6. 获取顶点的所有邻居（返回 <邻居, 权重> 列表）
        std::vector<std::pair<T, int>> get_neighbors(const T &vertex) const
        {
            if (adj_list_.find(vertex) == adj_list_.end())
            {
                throw std::invalid_argument("Vertex does not exist");
            }
            return adj_list_.at(vertex);
        }

        // 7. 获取所有顶点
        std::unordered_set<T> get_vertices() const
        {
            std::unordered_set<T> vertices;
            for (const auto &pair : adj_list_)
            {
                vertices.insert(pair.first);
            }
            return vertices;
        }

        // 8. 打印图（邻接表形式）
        void print_graph() const
        {
            for (const auto &pair : adj_list_)
            {
                std::cout << pair.first << ": ";
                for (size_t i = 0; i < pair.second.size(); ++i)
                {
                    const auto &neighbor = pair.second[i];
                    std::cout << "(" << neighbor.first << ", " << neighbor.second << ")";
                    if (i != pair.second.size() - 1)
                    {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
        }

        // 获取图的类型（是否为有向图）
        bool is_directed() const
        {
            return is_directed_;
        }

        // ========== 扩展功能 ==========
        // 9. 获取顶点数量
        size_t get_vertex_count() const
        {
            return adj_list_.size();
        }

        // 10. 获取边数量
        size_t get_edge_count() const
        {
            size_t count = 0;
            for (const auto &pair : adj_list_)
            {
                count += pair.second.size();
            }
            // 无向图的边会被双向存储，需要除以2
            return is_directed_ ? count : count / 2;
        }

        // 11. 深度优先遍历（DFS）- 递归版
        std::vector<T> dfs(const T &start_vertex) const
        {
            if (adj_list_.find(start_vertex) == adj_list_.end())
            {
                throw std::invalid_argument("Start vertex does not exist");
            }

            std::unordered_set<T> visited;
            std::vector<T> result;
            dfs_helper(start_vertex, visited, result);
            return result;
        }

        // 12. 广度优先遍历（BFS）
        std::vector<T> bfs(const T &start_vertex) const
        {
            if (adj_list_.find(start_vertex) == adj_list_.end())
            {
                throw std::invalid_argument("Start vertex does not exist");
            }

            std::unordered_set<T> visited;
            std::vector<T> result;
            std::queue<T> q;

            q.push(start_vertex);
            visited.insert(start_vertex);

            while (!q.empty())
            {
                T current = q.front();
                q.pop();
                result.push_back(current);

                // 遍历当前顶点的所有邻居
                for (const auto &neighbor : adj_list_.at(current))
                {
                    if (visited.find(neighbor.first) == visited.end())
                    {
                        visited.insert(neighbor.first);
                        q.push(neighbor.first);
                    }
                }
            }
            return result;
        }

        // 13. Dijkstra算法：获取从起点到所有顶点的最短路径
        std::unordered_map<T, int> dijkstra(const T &start_vertex) const
        {
            if (adj_list_.find(start_vertex) == adj_list_.end())
            {
                throw std::invalid_argument("Start vertex does not exist");
            }

            // 存储起点到各顶点的最短距离，初始为无穷大
            std::unordered_map<T, int> distances;
            for (const auto &pair : adj_list_)
            {
                distances[pair.first] = INT_MAX;
            }
            distances[start_vertex] = 0;

            // 优先队列：<距离, 顶点>，按距离升序排列
            std::priority_queue<std::pair<int, T>,
                                std::vector<std::pair<int, T>>,
                                std::greater<std::pair<int, T>>>
                pq;
            pq.push({0, start_vertex});

            while (!pq.empty())
            {
                auto [current_dist, current_vertex] = pq.top();
                pq.pop();

                // 如果当前距离大于已记录的最短距离，跳过
                if (current_dist > distances[current_vertex])
                {
                    continue;
                }

                // 遍历邻居，更新距离
                for (const auto &[neighbor, weight] : adj_list_.at(current_vertex))
                {
                    int new_dist = current_dist + weight;
                    if (new_dist < distances[neighbor])
                    {
                        distances[neighbor] = new_dist;
                        pq.push({new_dist, neighbor});
                    }
                }
            }

            return distances;
        }

        // 14. 判断图是否有环（支持有向/无向图）
        bool has_cycle() const
        {
            std::unordered_set<T> visited;
            std::unordered_set<T> rec_stack; // 递归栈（用于有向图检测环）

            for (const auto &pair : adj_list_)
            {
                const T &vertex = pair.first;
                if (visited.find(vertex) == visited.end())
                {
                    if (is_directed_)
                    {
                        if (has_cycle_directed(vertex, visited, rec_stack))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        if (has_cycle_undirected(vertex, visited, T()))
                        { // T() 表示无父节点
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        // 15. 获取两个顶点之间的最短路径（基于Dijkstra）
        std::vector<T> get_shortest_path(const T &start, const T &end) const
        {
            auto distances = dijkstra(start);
            if (distances[end] == INT_MAX)
            {
                return {}; // 无路径
            }

            // 回溯找路径
            std::vector<T> path;
            T current = end;
            path.push_back(current);

            while (current != start)
            {
                for (const auto &[vertex, neighbors] : adj_list_)
                {
                    for (const auto &[neighbor, weight] : neighbors)
                    {
                        if (neighbor == current && distances[vertex] + weight == distances[current])
                        {
                            current = vertex;
                            path.push_back(current);
                            break;
                        }
                    }
                    if (current == start)
                        break;
                }
            }

            std::reverse(path.begin(), path.end());
            return path;
        }

    private:
        // 邻接表：key=顶点，value=该顶点的邻居列表（pair<邻居顶点, 边权重>）
        std::unordered_map<T, std::vector<std::pair<T, int>>> adj_list_;
        // 是否为有向图（false=无向图，true=有向图）
        bool is_directed_;

        // ========== 私有辅助函数 ==========
        // DFS递归辅助函数
        void dfs_helper(const T &current, std::unordered_set<T> &visited, std::vector<T> &result) const
        {
            visited.insert(current);
            result.push_back(current);

            // 遍历所有邻居
            for (const auto &neighbor : adj_list_.at(current))
            {
                if (visited.find(neighbor.first) == visited.end())
                {
                    dfs_helper(neighbor.first, visited, result);
                }
            }
        }

        // 有向图检测环的辅助函数
        bool has_cycle_directed(const T &current, std::unordered_set<T> &visited, std::unordered_set<T> &rec_stack) const
        {
            visited.insert(current);
            rec_stack.insert(current);

            for (const auto &neighbor : adj_list_.at(current))
            {
                const T &next = neighbor.first;
                if (visited.find(next) == visited.end())
                {
                    if (has_cycle_directed(next, visited, rec_stack))
                    {
                        return true;
                    }
                }
                else if (rec_stack.find(next) != rec_stack.end())
                {
                    // 发现回边，存在环
                    return true;
                }
            }

            rec_stack.erase(current);
            return false;
        }

        // 无向图检测环的辅助函数（parent为父节点，避免误判）
        bool has_cycle_undirected(const T &current, std::unordered_set<T> &visited, const T &parent) const
        {
            visited.insert(current);

            for (const auto &neighbor : adj_list_.at(current))
            {
                const T &next = neighbor.first;
                if (visited.find(next) == visited.end())
                {
                    if (has_cycle_undirected(next, visited, current))
                    {
                        return true;
                    }
                }
                else if (next != parent)
                {
                    // 访问过且不是父节点，存在环
                    return true;
                }
            }

            return false;
        }
    };
}
#endif // GRAPH_H
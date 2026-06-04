#include "Dijkstra.hpp"

#include <queue>
#include <limits>
#include <utility>
using namespace std; 

vector<double> dijkstra(const Graph& graph, int source) {
    const double INF = numeric_limits<double>::infinity();

    int n = graph.size();
    vector<double> distance(n, INF);

    using QueueElement = pair<double, int>;
    priority_queue<
        QueueElement,
        vector<QueueElement>,
        greater<QueueElement>
    > queue;

    distance[source] = 0.0;
    queue.push({ 0.0, source });

    while (!queue.empty()) {
        double currentDistance = queue.top().first;
        int vertex = queue.top().second;
        queue.pop();

        if (currentDistance > distance[vertex]) {
            continue;
        }

        for (const Edge& edge : graph.neighbors(vertex)) {
            int to = edge.to;
            double newDistance = currentDistance + edge.weight;

            if (newDistance < distance[to]) {
                distance[to] = newDistance;
                queue.push({ newDistance, to });
            }
        }
    }

    return distance;
}
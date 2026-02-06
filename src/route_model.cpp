#include "route_model.h"

RouteModel::RouteModel(Model& model, double step)
    : m_Model(model)
{
    double minLat = model.GetMinLat();
    double maxLat = model.GetMaxLat();
    double minLon = model.GetMinLon();
    double maxLon = model.GetMaxLon();

    BuildGrid(minLat, maxLat, minLon, maxLon, step);
    ConnectAirports(model);
}

void RouteModel::BuildGrid(double minLat, double maxLat,
                           double minLon, double maxLon,
                           double step)
{
    int rows = static_cast<int>((maxLat - minLat) / step) + 1;
    int cols = static_cast<int>((maxLon - minLon) / step) + 1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double lat = minLat + r * step;
            double lon = minLon + c * step;
            nodes.emplace_back(lat, lon);
        }
    }

    ConnectGridNeighbors(rows, cols);
}

void RouteModel::ConnectGridNeighbors(int rows, int cols)
{
    auto index = [cols](int r, int c) {
        return r * cols + c;
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {

            int current = index(r, c);

            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {

                    if (dr == 0 && dc == 0) continue;

                    int nr = r + dr;
                    int nc = c + dc;

                    if (nr >= 0 && nr < rows &&
                        nc >= 0 && nc < cols) {

                        int neighbor = index(nr, nc);
                        nodes[current].neighbors.push_back(&nodes[neighbor]);
                    }
                }
            }
        }
    }
}

void RouteModel::ConnectAirports(const Model& model)
{
    const auto& airportIndices = model.GetAirportIndices();
    const auto& osmNodes = model.GetNodes();

    for (int idx : airportIndices) {

        double lat = osmNodes[idx].lat;
        double lon = osmNodes[idx].lon;

        nodes.emplace_back(lat, lon);
        Node* airportNode = &nodes.back();

        // connect airport to nearest grid node
        Node* closest = FindClosestNode(lat, lon);

        airportNode->neighbors.push_back(closest);
        closest->neighbors.push_back(airportNode);
    }
}

RouteModel::Node* RouteModel::FindClosestNode(double lat, double lon)
{
    Node temp(lat, lon);

    double minDist = std::numeric_limits<double>::max();
    Node* closest = nullptr;

    for (auto& node : nodes) {
        double dist = temp.distance(node);
        if (dist < minDist) {
            minDist = dist;
            closest = &node;
        }
    }

    return closest;
}

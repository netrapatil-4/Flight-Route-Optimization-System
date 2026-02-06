#ifndef ROUTE_MODEL_H
#define ROUTE_MODEL_H

#include <vector>
#include <cmath>
#include <limits>
#include "model.h"

class RouteModel {

public:

    struct Node {
        double lat;
        double lon;

        Node* parent = nullptr;
        double g_value = 0.0;
        double h_value = std::numeric_limits<double>::max();
        bool visited = false;

        std::vector<Node*> neighbors;

        Node(double latitude, double longitude)
            : lat(latitude), lon(longitude) {}

        // Haversine distance (great-circle)
        double distance(const Node& other) const {
            const double R = 6371.0; // Earth radius (km)

            double lat1 = lat * M_PI / 180.0;
            double lat2 = other.lat * M_PI / 180.0;
            double dLat = (other.lat - lat) * M_PI / 180.0;
            double dLon = (other.lon - lon) * M_PI / 180.0;

            double a = sin(dLat/2) * sin(dLat/2) +
                       cos(lat1) * cos(lat2) *
                       sin(dLon/2) * sin(dLon/2);

            double c = 2 * atan2(sqrt(a), sqrt(1 - a));
            return R * c;
        }
    };

    RouteModel(Model& model, double step);

    Node* FindClosestNode(double lat, double lon);

    std::vector<Node> nodes;
    std::vector<Node> path;

private:

    void BuildGrid(double minLat, double maxLat,
                   double minLon, double maxLon,
                   double step);

    void ConnectGridNeighbors(int rows, int cols);

    void ConnectAirports(const Model& model);

    Model& m_Model;
};

#endif

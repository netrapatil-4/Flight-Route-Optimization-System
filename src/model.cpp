#include "model.h"
#include "pugixml.hpp"
#include <iostream>
#include <string_view>
#include <unordered_set>

Model::Model(const std::vector<std::byte>& xml) {
    LoadData(xml);
}

void Model::LoadData(const std::vector<std::byte>& xml) {

    using namespace pugi;

    xml_document doc;
    if (!doc.load_buffer(xml.data(), xml.size()))
        throw std::logic_error("Failed to parse XML file");

    // Extract bounds
    if (auto bounds = doc.select_nodes("/osm/bounds"); !bounds.empty()) {
        auto node = bounds.first().node();
        m_MinLat = atof(node.attribute("minlat").as_string());
        m_MaxLat = atof(node.attribute("maxlat").as_string());
        m_MinLon = atof(node.attribute("minlon").as_string());
        m_MaxLon = atof(node.attribute("maxlon").as_string());
    }

    // Store all nodes (lat/lon)
    std::unordered_map<std::string, int> node_id_map;

    for (const auto& node : doc.select_nodes("/osm/node")) {

        node_id_map[node.node().attribute("id").as_string()] = m_Nodes.size();

        Node new_node;
        new_node.lat = atof(node.node().attribute("lat").as_string());
        new_node.lon = atof(node.node().attribute("lon").as_string());

        m_Nodes.push_back(new_node);
    }

    // Extract aerodromes (airports)
    for (const auto& way : doc.select_nodes("/osm/way")) {

        auto way_node = way.node();

        bool isAerodrome = false;

        for (auto child : way_node.children("tag")) {

            std::string_view key = child.attribute("k").as_string();
            std::string_view value = child.attribute("v").as_string();

            if (key == "aeroway" && value == "aerodrome") {
                isAerodrome = true;
                break;
            }
        }

        if (isAerodrome) {

            for (auto nd : way_node.children("nd")) {
                std::string ref = nd.attribute("ref").as_string();
                if (node_id_map.count(ref)) {
                    m_AirportNodeIndices.push_back(node_id_map[ref]);
                }
            }
        }
    }
}

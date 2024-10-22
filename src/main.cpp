#include "standard.hpp"

std::size_t n, m, source, destination;
std::vector<std::pair<double, double>> points; // pairs of (longitude, latitude)
std::vector<std::map<std::size_t, double>> neighbors;

double convert(double angle)
{
    return angle * M_PI / 180.0;
}

double haversine(
    double lon1, double lat1,
    double lon2, double lat2)
{
    static const double EARTH_RADIUS = 6371.0;

    double dlon = convert(lon2 - lon1);
    double dlat = convert(lat2 - lat1);

    double a = std::pow(std::sin(dlat / 2.0), 2) + std::cos(convert(lat1)) * std::cos(convert(lat2)) * std::pow(std::sin(dlon / 2.0), 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    return EARTH_RADIUS * c;
}

double haversine(
    std::size_t first, std::size_t second)
{
    return haversine(points[first].first, points[first].second, points[second].first, points[second].second);
}

class search_pack
{
public:
    const std::size_t index;
    const std::shared_ptr<search_pack> parent;
    const double distance_to_src;
    const double estimated_to_dest;

    search_pack(
        std::size_t index,
        std::shared_ptr<search_pack> parent,
        double distance_to_src,
        double estimated_to_dest)
        : index(index),
          parent(parent),
          distance_to_src(distance_to_src),
          estimated_to_dest(estimated_to_dest)
    {
    }

    bool operator<(const search_pack &other) const
    {
        return distance_to_src + estimated_to_dest < other.distance_to_src + other.estimated_to_dest;
    }
};

std::vector<std::size_t> search()
{
    std::vector<bool> discovered(n);
    std::set<std::shared_ptr<search_pack>> queue;

    queue.insert(std::make_shared<search_pack>(source, nullptr, 0.0, haversine(source, destination)));

    while (!queue.empty())
    {
        auto pack_ptr = *queue.begin();
        queue.erase(queue.begin());

        if (!discovered[pack_ptr->index])
        {
            discovered[pack_ptr->index] = true;
            if (pack_ptr->index == destination)
            {
                std::vector<std::size_t> result;
                auto ptr = pack_ptr;
                while (ptr != nullptr)
                {
                    result.push_back(ptr->index);
                    ptr = ptr->parent;
                }

                std::reverse(result.begin(), result.end());
                std::cerr << "Found route with distance = " << pack_ptr->distance_to_src << " km" << std::endl;
                return result;
            }

            for (auto &[neighbor, distance] : neighbors[pack_ptr->index])
            {
                queue.insert(std::make_shared<search_pack>(neighbor, pack_ptr, pack_ptr->distance_to_src + distance, haversine(neighbor, destination)));
            }
        }
    }

    return std::vector<std::size_t>{source};
}

int main()
{
    long long source_index, destination_index;
    std::cin >> n >> m >> source_index >> destination_index;

    std::map<long long, std::size_t> index_mapping;
    std::vector<long long> reverse_index_mapping;
    for (std::size_t i = 0; i < n; i++)
    {
        long long index;
        std::cin >> index;
        index_mapping[index] = i;
        reverse_index_mapping.push_back(index);

        double lon, lat;
        std::cin >> lon >> lat;
        points.emplace_back(lon, lat);
    }

    source = index_mapping[source_index];
    destination = index_mapping[destination_index];

    neighbors.resize(n);
    for (std::size_t i = 0; i < m; i++)
    {
        long long u, v;
        std::cin >> u >> v;

        std::size_t u_index = index_mapping[u];
        std::size_t v_index = index_mapping[v];

        double distance = haversine(points[u_index].first, points[u_index].second, points[v_index].first, points[v_index].second);
        neighbors[u_index][v_index] = neighbors[v_index][u_index] = distance;
    }

    auto path = search();
    for (auto &index : path)
    {
        std::cout << reverse_index_mapping[index] << " ";
    }
    std::cout << std::endl;

    return 0;
}
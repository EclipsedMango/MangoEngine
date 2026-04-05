
#include "PropertyHolder.h"
#include "PropertyRegistry.h"

static inline bool sRegisteredPropertyHolder = [] {
    PropertyRegistry::Register("PropertyHolder", []() -> PropertyHolder* {
        return new PropertyHolder();
    });
    return true;
}();

PropertyValue PropertyHolder::DeserializePropertyValue(const fkyaml::node &val) {
    if (val.is_null()) {
        return std::string(""); // null yaml value = empty string
    }

    if (val.is_boolean()) {
        return val.get_value<bool>();
    }

    if (val.is_integer()) {
        return val.get_value<int>();
    }

    if (val.is_float_number()) {
        return static_cast<float>(val.get_value<double>());
    }

    if (val.is_string()) {
        return val.get_value<std::string>();
    }

    if (val.is_mapping() && val.contains("z")) {
        auto getFloat = [](const fkyaml::node& n) -> float {
            if (n.is_null()) return 0.0f;
            if (n.is_integer()) return static_cast<float>(n.get_value<int>());
            return static_cast<float>(n.get_value<double>());
        };

        return glm::vec3(getFloat(val["x"]), getFloat(val["y"]), getFloat(val["z"]));
    }

    if (val.is_mapping() && val.contains("x") && val.contains("y")) {
        return glm::vec2(
            val["x"].get_value<float>(),
            val["y"].get_value<float>()
        );
    }

    if (val.is_mapping() && val.contains("property_type")) {
        const std::string type = val["property_type"].get_value<std::string>();

        std::shared_ptr<PropertyHolder> holder;
        try {
            holder = std::shared_ptr<PropertyHolder>(PropertyRegistry::Create(type));
        } catch (...) {
            holder = std::make_shared<PropertyHolder>();
        }

        for (auto& [subKey, subVal] : val.get_value<fkyaml::ordered_map<std::string, fkyaml::node>>()) {
            if (subKey == "property_type") continue;
            PropertyValue deserialized = DeserializePropertyValue(subVal);
            try {
                holder->Set(subKey, deserialized);
            } catch (const std::runtime_error&) {
                holder->AddProperty(subKey, [deserialized] { return deserialized; }, {});
            }
        }
        return holder;
    }

    if (val.is_mapping()) {
        auto holder = std::make_shared<PropertyHolder>();
        for (auto& [subKey, subVal] : val.get_value<fkyaml::ordered_map<std::string, fkyaml::node>>()) {
            PropertyValue captured = DeserializePropertyValue(subVal);
            holder->AddProperty(subKey, [captured] { return captured; }, {});
        }
        return holder;
    }

    return 0;
}

fkyaml::node PropertyHolder::SerializeProperty(PropertyValue property) {
    fkyaml::node ret;
    std::visit([&]<typename T0>(T0&& val) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, float>) {
            ret = static_cast<float>(val);
        }

        if constexpr (std::is_same_v<T, int>) {
            ret = static_cast<int>(val);
        }

        if constexpr (std::is_same_v<T, bool>) {
            ret = static_cast<bool>(val);
        }

        if constexpr (std::is_same_v<T, std::string>) {
            ret = static_cast<std::string>(val);
        }

        if constexpr (std::is_same_v<T, glm::vec2>) {
            glm::vec2 v = val;
            ret = fkyaml::node { {"x", v.x}, {"y", v.y} };
        }

        if constexpr (std::is_same_v<T, glm::vec3>) {
            glm::vec3 v = val;
            ret = fkyaml::node { {"x", v.x}, {"y", v.y}, {"z", v.z} };
        }

        if constexpr (std::is_same_v<T, std::shared_ptr<PropertyHolder>>) {
            fkyaml::node properties = fkyaml::node::mapping();
            properties["property_type"] = val->GetPropertyHolderType();
            for (const auto& [key, prop] : val->GetProperties()) {
                properties[key] = SerializeProperty(prop.getter());
            }
            ret = properties;
        }
    }, property);

    return ret;
}
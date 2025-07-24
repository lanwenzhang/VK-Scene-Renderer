#include "scene.h"


namespace lzvk::loader {

	int addNode(Scene& scene, int parent, int level) {

		const int node = static_cast<int>(scene.hierarchy.size());

		scene.localTransform.push_back(glm::mat4(1.0f));
		scene.globalTransform.push_back(glm::mat4(1.0f));
		Hierarchy h;
		h.parent = parent;
		h.level = level;
		scene.hierarchy.push_back(h);

		if (parent > -1) {

			auto& parentNode = scene.hierarchy[parent];
			if (parentNode.firstChild == -1) {

				parentNode.firstChild = node;
				scene.hierarchy[node].lastSibling = node;
			}
			else {

				int last = scene.hierarchy[parentNode.firstChild].lastSibling;
				scene.hierarchy[last].nextSibling = node;
				scene.hierarchy[parentNode.firstChild].lastSibling = node;

			}
		}

		return node;
	}

	glm::mat4 toMat4(const aiMatrix4x4& a) {

		return glm::mat4(
			a.a1, a.b1, a.c1, a.d1,
			a.a2, a.b2, a.c2, a.d2,
			a.a3, a.b3, a.c3, a.d3,
			a.a4, a.b4, a.c4, a.d4
		);
	}

	bool recalculateGlobalTransforms(Scene& scene) {

		if (scene.hierarchy.empty()) return false;

		scene.globalTransform[0] = scene.localTransform[0];
		for (size_t i = 1; i < scene.hierarchy.size(); ++i) {

			const int parent = scene.hierarchy[i].parent;
			if (parent >= 0) {
				scene.globalTransform[i] = scene.globalTransform[parent] * scene.localTransform[i];
			}
			else {
				scene.globalTransform[i] = scene.localTransform[i];
			}
		}

		return true;
	}

	void markAsChanged(Scene& scene, int node) {

		std::set<int> stack;
		stack.insert(node);

		while (!stack.empty()) {
			int current = *stack.begin();
			stack.erase(stack.begin());

			for (int c = scene.hierarchy[current].firstChild; c != -1; c = scene.hierarchy[c].nextSibling) {
				stack.insert(c);
			}
		}
	}

	std::string getNodeName(const Scene& scene, int node) {
		auto it = scene.nameForNode.find(node);
		if (it != scene.nameForNode.end() && it->second < scene.nodeNames.size()) {
			return scene.nodeNames[it->second];
		}
		return std::string();
	}

    void saveScene(const std::string& path, const Scene& scene) {
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) {
            printf("Failed to open file %s for writing\n", path.c_str());
            return;
        }

        uint64_t numNodes = scene.hierarchy.size();
        fwrite(&numNodes, sizeof(numNodes), 1, f);
        if (numNodes > 0) {
            fwrite(scene.hierarchy.data(), sizeof(Hierarchy), numNodes, f);
            fwrite(scene.localTransform.data(), sizeof(glm::mat4), numNodes, f);
            fwrite(scene.globalTransform.data(), sizeof(glm::mat4), numNodes, f);
        }

        auto saveMap = [](FILE* f, const std::unordered_map<uint32_t, uint32_t>& map) {
            uint64_t count = map.size();
            fwrite(&count, sizeof(count), 1, f);
            for (const auto& [k, v] : map) {
                fwrite(&k, sizeof(k), 1, f);
                fwrite(&v, sizeof(v), 1, f);
            }
            };

        saveMap(f, scene.meshForNode);
        saveMap(f, scene.materialForNode);
        saveMap(f, scene.nameForNode);

        auto saveStringList = [](FILE* f, const std::vector<std::string>& list) {
            uint64_t count = list.size();
            fwrite(&count, sizeof(count), 1, f);
            for (const auto& s : list) {
                uint64_t len = s.length();
                fwrite(&len, sizeof(len), 1, f);
                fwrite(s.c_str(), 1, len, f);
            }
            };

        saveStringList(f, scene.nodeNames);
        saveStringList(f, scene.materialNames);

        uint64_t numDrawData = scene.drawDataArray.size();
        fwrite(&numDrawData, sizeof(numDrawData), 1, f);
        if (numDrawData > 0) {
            fwrite(scene.drawDataArray.data(), sizeof(DrawData), numDrawData, f);
        }

        fclose(f);
        printf("Scene saved to %s\n", path.c_str());
    }

    bool loadScene(const std::string& path, Scene& scene) {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return false;

        uint64_t numNodes;
        fread(&numNodes, sizeof(numNodes), 1, f);
        scene.hierarchy.resize(numNodes);
        scene.localTransform.resize(numNodes);
        scene.globalTransform.resize(numNodes);
        if (numNodes > 0) {
            fread(scene.hierarchy.data(), sizeof(Hierarchy), numNodes, f);
            fread(scene.localTransform.data(), sizeof(glm::mat4), numNodes, f);
            fread(scene.globalTransform.data(), sizeof(glm::mat4), numNodes, f);
        }

        auto loadMap = [](FILE* f, std::unordered_map<uint32_t, uint32_t>& map) {
            uint64_t count;
            fread(&count, sizeof(count), 1, f);
            for (uint64_t i = 0; i < count; i++) {
                uint32_t k, v;
                fread(&k, sizeof(k), 1, f);
                fread(&v, sizeof(v), 1, f);
                map[k] = v;
            }
            };

        loadMap(f, scene.meshForNode);
        loadMap(f, scene.materialForNode);
        loadMap(f, scene.nameForNode);

        auto loadStringList = [](FILE* f, std::vector<std::string>& list) {
            uint64_t count;
            fread(&count, sizeof(count), 1, f);
            list.resize(count);
            for (uint64_t i = 0; i < count; i++) {
                uint64_t len;
                fread(&len, sizeof(len), 1, f);
                list[i].resize(len);
                fread(&list[i][0], 1, len, f);
            }
            };

        loadStringList(f, scene.nodeNames);
        loadStringList(f, scene.materialNames);

        uint64_t numDrawData;
        fread(&numDrawData, sizeof(numDrawData), 1, f);
        scene.drawDataArray.resize(numDrawData);
        if (numDrawData > 0) {
            fread(scene.drawDataArray.data(), sizeof(DrawData), numDrawData, f);
        }


        fclose(f);
        printf("Scene loaded from %s\n", path.c_str());
        return true;
    }


}


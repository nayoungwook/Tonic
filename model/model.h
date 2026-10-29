#pragma once

#include <string>
#include <vector>

class Model {
public:
	explicit Model(const std::string &path);

	const std::vector<float> &get_vertices() const;
	const std::vector<unsigned> &get_indices() const;

private:
	void load_model(const std::string &path);

	std::vector<float> vertices;
	std::vector<unsigned> indices;
};

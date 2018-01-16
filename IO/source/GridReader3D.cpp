#include <IO/GridReader3D.hpp>

GridReader3D::GridReader3D(std::string&& filePath) : GridReader(std::move(filePath)) {
	this->gridData.dimension = 3;
	this->readNodes();
	this->readPhysicalEntities();
	this->readElements();
	this->addElements();
}

void GridReader3D::readPhysicalEntities() {
	this->file.seekg(0, std::ios::beg); 
	while (strcmp(this->buffer, "$PhysicalNames") && !this->file.eof()) this->file >> this->buffer;
	if (this->file.eof()) throw std::runtime_error("There is no Physical Entities data in the grid file");
	this->file >> this->numberOfPhysicalEntities;
	std::vector<int> entitiesTypes;			int type;
	std::vector<int> entitiesNumbers;		int number;
	std::vector<std::string> entitiesNames; std::string name;
	for (int i = 0; i < this->numberOfPhysicalEntities; i++) {
		file >> type >> number >> name;
		number--;
		name.erase(name.begin()); name.erase(name.end()-1);
 		entitiesTypes.push_back(type);
		entitiesNumbers.push_back(number);
		entitiesNames.push_back(name);
	}

	std::vector<int> geometryNumbers, boundaryNumbers;
	for (int i = 0; i < this->numberOfPhysicalEntities; i++) {
		if (entitiesTypes[i] == 2) {
			boundaryNumbers.push_back(entitiesNumbers[i]);
		}
		else if (entitiesTypes[i] == 3) {
			geometryNumbers.push_back(entitiesTypes[i]);
		}
		else {
			throw std::runtime_error("Non supported physical entity found");
		}
	}
	if (geometryNumbers.size() != 1) throw std::runtime_error("One and only one geometry supported");
	this->geometryNumber = geometryNumbers[0];

	this->gridData.boundaries.resize(boundaryNumbers.size());
	for (unsigned int i = 0; i < boundaryNumbers.size(); i++) {
		this->gridData.boundaries[i].name = entitiesNames[boundaryNumbers[i]];
	}
}

void GridReader3D::addElements() {
	for (unsigned int i = 0; i < this->physicalEntitiesElementIndices.size(); i++) {
		for (unsigned int j = 0; j < this->physicalEntitiesElementIndices[i].size(); j++) {
			int index = this->physicalEntitiesElementIndices[i][j];
			int type  = this->elements[index][0];
			if (type == 2) {
				std::vector<int>::const_iterator first = this->elements[index].cbegin() + 4;
				std::vector<int>::const_iterator last  = this->elements[index].cend();
				std::vector<int> triangle(first, last);
				for (unsigned int i = 0; i < triangle.size(); i++) triangle[i]--;
				this->gridData.boundaries[i].triangleConnectivity.emplace_back(std::move(triangle));
			}
			else if (type == 3) {
				std::vector<int>::const_iterator first = this->elements[index].cbegin() + 4;
				std::vector<int>::const_iterator last  = this->elements[index].cend();
				std::vector<int> quadrangle(first, last);
				for (unsigned int i = 0; i < quadrangle.size(); i++) quadrangle[i]--;
				this->gridData.boundaries[i].quadrangleConnectivity.emplace_back(std::move(quadrangle));
			}
			else if (type == 4) {
				std::vector<int>::const_iterator first = this->elements[index].cbegin() + 4;
				std::vector<int>::const_iterator last  = this->elements[index].cend();
				std::vector<int> tetrahedron(first, last);
				for (unsigned int i = 0; i < tetrahedron.size(); i++) tetrahedron[i]--;
				this->gridData.tetrahedronConnectivity.emplace_back(std::move(tetrahedron));
			}
			else if (type == 5) {
				std::vector<int>::const_iterator first = this->elements[index].cbegin() + 4;
				std::vector<int>::const_iterator last  = this->elements[index].cend();
				std::vector<int> hexahedron(first, last);
				for (unsigned int i = 0; i < hexahedron.size(); i++) hexahedron[i]--;
				this->gridData.hexahedronConnectivity.emplace_back(std::move(hexahedron));
			}
			else if (type == 7) {
				std::vector<int>::const_iterator first = this->elements[index].cbegin() + 4;
				std::vector<int>::const_iterator last  = this->elements[index].cend();
				std::vector<int> pyramid(first, last);
				for (unsigned int i = 0; i < pyramid.size(); i++) pyramid[i]--;
				this->gridData.pyramidConnectivity.emplace_back(std::move(pyramid));
			}
			else {
				throw std::runtime_error("Non supported element found");
			}
		}
	}
}

#include <CgnsInterface/CgnsFile2D.hpp>

CgnsFile2D::CgnsFile2D(const GridData& gridData, const std::string& folderPath) : 
	CgnsFile(gridData, folderPath) {
	this->coordinateIndices.resize(3);
	this->sectionIndices.resize(5);
	this->boundaryIndices.resize(4);
	this->numberOfNodes    = this->gridData.coordinates.size();
	this->numberOfElements = this->gridData.triangleConnectivity.size();
	this->cellDimension    = this->gridData.dimension;
	std::string folderName = this->folderPath + std::string("/") + std::to_string(numberOfNodes) + std::string("n_") + std::to_string(numberOfElements) + "e/"; createDirectory(folderName);
	this->fileName = folderName + std::string("Grid.cgns");
	cg_open(this->fileName.c_str(), CG_MODE_WRITE, &this->fileIndex);
}

void CgnsFile2D::writeBase() {
	cg_base_write(this->fileIndex, this->baseName.c_str(), this->cellDimension, this->physicalDimension, &this->baseIndex);
}

void CgnsFile2D::writeZone() {
	this->zoneSizes[0] = this->numberOfNodes;
	this->zoneSizes[1] = this->numberOfElements;
	this->zoneSizes[2] = 0;	
	cg_zone_write(this->fileIndex, this->baseIndex, this->zoneName.c_str(), &this->zoneSizes[0], Unstructured, &this->zoneIndex);
}

void CgnsFile2D::writeCoordinates() {
	double coordinatesX[this->numberOfNodes];
	double coordinatesY[this->numberOfNodes];
	double coordinatesZ[this->numberOfNodes];
	for (cgsize_t i = 0; i < this->numberOfNodes; i++) {
		coordinatesX[i] = this->gridData.coordinates[i][0];
		coordinatesY[i] = this->gridData.coordinates[i][1];
		coordinatesZ[i] = 0.0; 
	}
	cg_coord_write(this->fileIndex, this->baseIndex, this->zoneIndex, RealDouble, "CoordinateX", coordinatesX, &this->coordinateIndices[0]);
	cg_coord_write(this->fileIndex, this->baseIndex, this->zoneIndex, RealDouble, "CoordinateY", coordinatesY, &this->coordinateIndices[1]);
	cg_coord_write(this->fileIndex, this->baseIndex, this->zoneIndex, RealDouble, "CoordinateZ", coordinatesZ, &this->coordinateIndices[2]);
}

void CgnsFile2D::writeSections() {
	cgsize_t* connectivities = determine_array_1d<cgsize_t>(gridData.triangleConnectivity);
	for (unsigned int i = 0; i < gridData.triangleConnectivity.size()*gridData.triangleConnectivity[0].size(); i++) connectivities[i]++;
	cgsize_t elementStart = 1;
	cgsize_t elementEnd = numberOfElements;
	cg_section_write(this->fileIndex, this->baseIndex, this->zoneIndex, "Geometry", TRI_3, elementStart, elementEnd, zoneSizes[2], connectivities, &sectionIndices[0]);
	delete connectivities;

	for (unsigned int i = 0; i < gridData.boundaries.size(); i++) {
		elementStart = elementEnd + 1;
		elementEnd = elementStart + gridData.boundaries[i].lineConnectivity.size() - 1;
		connectivities = determine_array_1d<cgsize_t>(gridData.boundaries[i].lineConnectivity);
		for (unsigned int j = 0; j < gridData.boundaries[i].lineConnectivity.size()*gridData.boundaries[i].lineConnectivity[0].size(); j++) connectivities[j]++;
		cg_section_write(this->fileIndex, this->baseIndex, this->zoneIndex, this->gridData.boundaries[i].name.c_str(), BAR_2, elementStart, elementEnd, this->zoneSizes[2], connectivities, &this->sectionIndices[i+1]);
		delete connectivities;
	}
}

void CgnsFile2D::writeBoundaryConditions() {
	for (unsigned int i = 0; i < gridData.boundaries.size(); i++) {
		std::set<int> vertices;
		for (auto j = gridData.boundaries[i].lineConnectivity.cbegin(); j != gridData.boundaries[i].lineConnectivity.cend(); j++) {
			for (auto k = j->cbegin(); k != j->cend(); k++) {
				vertices.insert(*k+1);
			}
		}
		cgsize_t* indices = determine_array_1d<cgsize_t>(vertices); 
		cg_boco_write(this->fileIndex, this->baseIndex, this->zoneIndex, this->gridData.boundaries[i].name.c_str(), BCWall, PointList, vertices.size(), indices, &this->boundaryIndices[i]);
		delete indices;
	}
}

CgnsFile2D::~CgnsFile2D() {
	cg_close(this->fileIndex);
}

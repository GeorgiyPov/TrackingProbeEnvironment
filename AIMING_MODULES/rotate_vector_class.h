#ifndef ROTATE_VECTOR_CLASS_H
#define ROTATE_VECTOR_CLASS_H

#define WIN32_LEAN_AND_MEAN 1

#include <QDebug>
#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <Eigen/src/Core/Matrix.h>
#include <Eigen/src/Core/Matrix.h>
#include <interface_pass_coord.h>
#include <initializer_list>
#include <QFile>

//SIMPLE VECTOR ROTATION, CLASS IS NOT USED IN PROGRAM NOW, RAPLACED BY RotateVectorContainer
enum RotateAxisenum { X_AXIS = 0, Y_AXIS= 1 , Z_AXIS = 2};

class RotateVectorClass : public PassTwoCoordClass
{
public:
	RotateVectorClass(RotateVectorClass& RotateObject);
    RotateVectorClass(std::initializer_list<std::pair<RotateAxisenum,double>> RotationList) ;
	RotateVectorClass() 
	{
		RotateMatrix << 1, 0, 0,
					    0, 1, 0,
				        0, 0, 1;
		RotateVector << 0, 0, 0;
	};

	void AppendRotateOperation(std::pair<RotateAxisenum, double> RotateOperation);

	Eigen::Matrix< float, 3, 3 > RotateMatrix;
	Eigen::Matrix< float, 2, 2 > RotateMatrixPlane;
	Eigen::Matrix< float, 3, 1 > OutputVector;
	Eigen::Matrix< float, 3, 1 > RotateVector;
    
	double SystemScale = 1;

	std::vector<std::pair<RotateAxisenum, double>> RotateOperations;

    void LoadRotationFromFile(QString Filename);

	void SetInput(const QPair<double,double>& Coord);

	std::string RotationToString(std::vector<std::pair<RotateAxisenum,double>> Sequence);
	std::string MatrixToString();

	const QPair<double,double>& GetOutput();
	RotateVectorClass& Inverse();
	void operator=(RotateVectorClass && Matrix);
	void operator=(RotateVectorClass & Matrix);

	friend RotateVectorClass operator*(RotateVectorClass  Rotate1,RotateVectorClass  Rotate2);

};

#endif //ROTATE_VECTOR_CLASS_H

#include "rotate_vector_class.h"
#include <iomanip>

RotateVectorClass::RotateVectorClass(RotateVectorClass& RotateObject)
{
	this->RotateMatrix = RotateObject.RotateMatrix;
	this->RotateVector = RotateObject.RotateVector;
}



void RotateVectorClass::AppendRotateOperation(std::pair<RotateAxisenum, double> RotateOperation)
{
	Eigen::Matrix< float, 3, 3 > NewOperationMatrix ;
	RotateOperations.push_back(RotateOperation);

	NewOperationMatrix << 1, 0 ,0,
						  0, 1 ,0,
						  0, 0, 1;

	if (RotateOperation.first == X_AXIS)
	{
		NewOperationMatrix(1, 1) = cos(RotateOperation.second*M_PI/180.0);
		NewOperationMatrix(1, 2) = -sin(RotateOperation.second*M_PI/180.0);
		NewOperationMatrix(2, 1) = sin(RotateOperation.second*M_PI/180.0);
		NewOperationMatrix(2, 2) = cos(RotateOperation.second*M_PI/180.0);
	}

	if (RotateOperation.first == Y_AXIS)
	{
		NewOperationMatrix(0, 0) = cos(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(2, 0) = sin(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(0, 2) = -sin(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(2, 2) = cos(RotateOperation.second*M_PI / 180.0);
	}

	if (RotateOperation.first == Z_AXIS)
	{
		NewOperationMatrix(0, 0) = cos(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(0, 1) = -sin(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(1, 0) = sin(RotateOperation.second*M_PI / 180.0);
		NewOperationMatrix(1, 1) = cos(RotateOperation.second*M_PI / 180.0);
	}

	this->RotateMatrix = NewOperationMatrix*RotateMatrix;
}


void RotateVectorClass::SetInput(const QPair<double,double>& Coord)
{

	Eigen::Matrix<float, 3, 1> InputVector;
							   InputVector << Coord.first, Coord.second, 0;
    OutputVector = SystemScale * RotateMatrix * InputVector;
}

const QPair<double, double>& RotateVectorClass::GetOutput() 
{ 
	return QPair<double, double>(this->OutputVector(0, 0), this->OutputVector(1, 0)); 
}

RotateVectorClass& RotateVectorClass::Inverse()
{
	RotateMatrix = this->RotateMatrix.inverse().eval();
    return *this;
}


RotateVectorClass operator*(RotateVectorClass  Rotate1,RotateVectorClass  Rotate2)
{
Rotate1.RotateMatrix = Rotate1.RotateMatrix*Rotate2.RotateMatrix;
Rotate1.RotateVector = Rotate1.RotateVector + Rotate2.RotateVector;
return Rotate1;
}

void RotateVectorClass::operator=(RotateVectorClass & Matrix)
{
	this->RotateMatrix = Matrix.RotateMatrix;
	this->RotateVector = Matrix.RotateVector;
}

void RotateVectorClass::operator=(RotateVectorClass && Matrix)
{
	this->RotateMatrix = Matrix.RotateMatrix;
	this->RotateVector = Matrix.RotateVector;
}

RotateVectorClass::RotateVectorClass(std::initializer_list<std::pair<RotateAxisenum,double>> RotationList) 
{
	RotateMatrix << 1, 0, 0,
					0, 1, 0,
					0, 0, 1;
	RotateVector << 0, 0, 0;

	for(auto& Rotation : RotationList) this->AppendRotateOperation(Rotation);

};

void RotateVectorClass::LoadRotationFromFile(QString Filename)
{

	 //qDebug() << "ROTATE_VECTOR  TRY LOAD ROTATION FROM FILE: " << Filename;
     QFile data(Filename); data.open(QFile::ReadOnly); if(!data.isOpen()) return;

     QTextStream in_stream(&data);
	 QString header = in_stream.readLine();

     std::vector<double> ME; ME.resize(9);

	 for(auto& value: ME) in_stream >> value; 

	 RotateMatrix << ME[0], ME[1], ME[2],
					 ME[3], ME[4], ME[5],
					 ME[6], ME[7], ME[8];

in_stream.readLine(); in_stream.readLine(); in_stream.readLine(); in_stream.readLine();
//=================================================================
RotateOperations.clear();

QString axis_string;  RotateAxisenum axis = X_AXIS; double angle_degree = 0;

for(int n = 1; n <=3; n++)
{
in_stream >> axis_string; in_stream >> angle_degree;
axis = X_AXIS; if(axis_string == "y_axis"){axis = Y_AXIS;}; if(axis_string == "z_axis"){axis = Z_AXIS;}; 
RotateOperations.push_back(std::make_pair(axis,angle_degree));
}

	 in_stream.readLine(); in_stream.readLine();
	 in_stream >> header >> this->SystemScale; 

     RotateMatrixPlane = RotateMatrix.block<2,2>(0,0);

	 //qDebug() << "===================================================";
	 //qDebug() << " ROTATION " << RotationToString(RotateOperations).c_str() << "[ SCALE ] " << SystemScale;
	 //qDebug() << " MATRIX   " << Qt::endl << this->MatrixToString().c_str();
	 //qDebug() << "===================================================";
}


std::string RotateVectorClass::MatrixToString()
{
    std::ostringstream out; out.precision(5); out << this->RotateMatrix;
	return out.str();
}

std::string RotateVectorClass::RotationToString(std::vector<std::pair<RotateAxisenum,double>> Sequence)
{
    std::ostringstream out; out.precision(4);
	for(auto Rotation: Sequence)
	{
	  if(Rotation.first == X_AXIS)	out << " [ X_AXIS ]";
	  if(Rotation.first == Y_AXIS)	out << " [ Y_AXIS ]";
	  if(Rotation.first == Z_AXIS)	out << " [ Z_AXIS ]";

	  out <<std::setw(5)<< Rotation.second;
	}
	return out.str();
}

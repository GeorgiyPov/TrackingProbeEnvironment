#ifndef ROTATEOPERATIONCONTAINER_H
#define ROTATEOPERATIONCONTAINER_H

#include <SubstractPair.h>
#include <DebugStream.h>
#include <interface_pass_coord.h>
#include "RotateOptimization_global.h"

#undef slots
#include <torch/torch.h>
#define slots Q_SLOTS
#define TEST_DATA_COUNT 512

using namespace std;
using namespace torch::indexing;
enum RotateAxis {x_axis = 0, y_axis = 1, z_axis = 2};
class RotateOperationContainer;


class ROTATEOPTIMIZATION_EXPORT TestDataVectorsContainer
{
public:
	TestDataVectorsContainer();
	vector<QPair<double,double>> TestCoordVectorInput;
	QPair<double, double> GetTestCoord();
	std::vector<QPair<double, double>>::iterator CurrentTestCoord;
	void reset();
};


class ROTATEOPTIMIZATION_EXPORT AccumulateDataFilter : public PassTwoCoordClass
{
public:
	AccumulateDataFilter();
	void SetInput(const QPair<double,double>& Coord) override;
	const QPair<double, double>& GetOutput() override;
	std::pair<double,double> GetFirstOutput();
	std::pair<double,double> GetSecondOutput();
	void WaitCoord(QPair<double,double> coord);
	bool CheckCoordMatch(QPair<double, double> Coord,QPair<double,double> AimCoord);
	void reset();

	bool flag_filter_opencv = false;
	int avarage_window_size = 8;
	int accumulate_counter = 0;
	int channel_counter = 1;
	int pass_counter = 0;

	QPair<double,double> ZeroCoord = QPair<double,double>(0,0);
    QPair<double,double> InputCoord;
	QPair<double,double>  LastInputFirst;
	QPair<double,double>  WaitInputCoord;

	QPair<double,double> AvarageOutputFirst;
	QPair<double,double> AvarageOutputSecond;
	QPair<double,double> OutputFirstCenter = QPair<double,double>(0,0);
	QPair<double,double> OutputSecondCenter = QPair<double,double>(0,0);

	friend void operator>>(AccumulateDataFilter& Filter, RotateOperationContainer& RotateContainer);
    friend AccumulateDataFilter& operator>>(QPair<double, double> coord, AccumulateDataFilter& Filter);
};

class ROTATEOPTIMIZATION_EXPORT RotateDataMeasureengine 
{
public:
	RotateDataMeasureengine();
	TestDataVectorsContainer DataVectors;
	AccumulateDataFilter DataFilter;
	void SwitchToNextTestCoord();
	QPair<double,double> CurrentMeasureCoordAbs;
	QPair<double,double> CurrentRelativeCoord;
	void Reset();
	QPair<double,double>& GetWaitInputCoord() { return DataFilter.WaitInputCoord;};

	friend RotateDataMeasureengine& operator>>(QPair<double,double> coord, RotateDataMeasureengine& Measureengine);
	friend RotateDataMeasureengine& operator>>(std::pair<double,double> coord, RotateDataMeasureengine& Measureengine);
	friend void operator>>(RotateDataMeasureengine& Measureengine, RotateOperationContainer& RotateContainer);
};

class ROTATEOPTIMIZATION_EXPORT RotateOperationContainer : public PassTwoCoordClass
{
public:
	RotateOperationContainer();
	RotateOperationContainer(const RotateOperationContainer& CopyContainer);

public:
	vector<pair<double,double>> input_to_optimize_rotation;
	vector<pair<double,double>> output_to_optimize_rotation;
	RotateDataMeasureengine MeasureFilter;

	void Reset();
	void CopyRotation(const RotateOperationContainer& CopyContainer);
  bool IsDataFull() { return output_to_optimize_rotation.size() == TEST_DATA_COUNT; } ;

    void SetRotateMatrix(torch::Tensor RotateMatrix);
	void SetInput(const QPair<double,double>& Coord) override;
	const QPair<double,double>& GetOutput() override;
	std::vector<float> GetOutputVector();
	void Inverse();
	double CalcVirtualZComponent(QPair<double,double> InputCoord);
  double CalcTransformationScale(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output);

	void AppendOperation(pair<RotateAxis,double> Operation);
	void AppendInputData(pair<double,double> test_coord);


	torch::Tensor ConvertInput(torch::Tensor& Input);
	torch::Tensor FitToTestVectors(torch::Tensor test_input, torch::Tensor test_output, float gamma, float speed, int number_iteration);
	torch::Tensor FitToTestVectors(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output, float gamma, float speed, int number_iteration);

	void FindArbitraryRotationToVectors(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output);
    pair<torch::Tensor,torch::Tensor> ConvertInputCoord(vector<pair<double,double>> test_input ,vector<pair<double,double>> test_output, bool z_signs_inverse);
	
//	void operator=(RotateOperationContainer & Rotation);
//	void operator=(RotateOperationContainer && Rotation);
	int counter_test = 0;
	bool RotationValid = true;
	float system_transform_scale = 1;

	bool isValid();


	void SaveMeasureDataToFile(QString Filename);
	void LoadRotationFromFile(QString Filename);


	string RotationToString();
	string RotationToString(vector<pair<RotateAxis,double>> Sequence);
	string MatrixToString();
private:
	void UpdateCommonMatrix(); // utilites to get rotate matrix from sequence angle rotate operations
    void ConvertOperationToMatrix(pair<RotateAxis,torch::Tensor>& Operation, torch::Tensor& Matrix);

	vector<pair<RotateAxis,double>>        RotateOperationSequense;  // rotate sequence in angle degree
	vector<pair<RotateAxis,torch::Tensor>> RotateOperationSequenseTensor; // rotate sequence in radian to calculate
	vector<torch::Tensor>                  RotateOperationMatrixSequense; // rotate sequence in matrix

    std::vector<float> OutputCoordVector = {0,0,0};
	torch::Tensor OutputCoord;
	torch::Tensor InputCoord;
	//=====================================================
	double pixel_scale_remote_to_local = 0; // remote it is output coord
	//=====================================================

	float rotate_matrix[9] = {0};
	torch::Tensor CommonMatrix; 
	torch::Tensor CommonSubMatrix;
	torch::Tensor CommonSubMatrixInverse;
	torch::Tensor CommonMatrixInverse;

	bool is_rotation_inverse = false;
	bool Requires_Grad = false;


	//auxilary tensors to synthesis rotate matrix from rotate angles
	torch::Tensor x_sin;
	torch::Tensor x_cos;
	torch::Tensor x_rotate_matrix_empty;

	torch::Tensor y_sin;
	torch::Tensor y_cos;
	torch::Tensor y_rotate_matrix_empty;

	torch::Tensor z_sin;
	torch::Tensor z_cos;
	torch::Tensor z_rotate_matrix_empty;
	torch::Tensor ones_matrix;


};

#endif //ROTATEOPERATIONCONTAINER_H

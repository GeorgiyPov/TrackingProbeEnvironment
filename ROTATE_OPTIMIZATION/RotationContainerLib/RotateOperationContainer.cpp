#include "RotateOperationContainer.h"
#include <DebugStream.h>
#include <iostream>
#include <QDebug>
#include <QFile>
static const char* TAG = "[ ROTATION ]";

TestDataVectorsContainer::TestDataVectorsContainer()
{
	TestCoordVectorInput.resize(TEST_DATA_COUNT+1);
	TestCoordVectorInput[0] = QPair<double,double>(0,0);
	int NumberPoints = TestCoordVectorInput.size();
	int CurrentPoint = 0;
	int Radius = 300;
	auto GenerateCircle = [NumberPoints,CurrentPoint,Radius]() mutable -> QPair<double,double>
	{
        double current_angle = CurrentPoint*2*M_PI/NumberPoints;
		QPair<double,double> PointOnCircle; PointOnCircle.first = Radius*std::cos(current_angle); 
		                                    PointOnCircle.second = Radius*std::sin(current_angle);
		CurrentPoint++;
		return PointOnCircle;

	};
    std::generate(TestCoordVectorInput.begin()+1,TestCoordVectorInput.end(),GenerateCircle);
  
    CurrentTestCoord = TestCoordVectorInput.begin();
}

void TestDataVectorsContainer::reset() 
{ 
	CurrentTestCoord = TestCoordVectorInput.begin(); 
}
const QPair<double,double>& AccumulateDataFilter::GetOutput() { return LastInputFirst; }

void AccumulateDataFilter::SetInput(const QPair<double,double>& Coord)
{
	InputCoord = Coord;
	if(WaitInputCoord != ZeroCoord)
	{
		if(channel_counter == 1)
		{
			InputCoord.first = Coord.first - OutputFirstCenter.first; InputCoord.second = Coord.second - OutputFirstCenter.second;  
            //qDebug()<< TAG << " WAIT REL: " << WaitInputCoord.first << WaitInputCoord.second
	        //        << TAG << " INPUT   : " << Coord.first << Coord.second;

			if(CheckCoordMatch(Coord,WaitInputCoord))
			{
				AvarageOutputFirst.first += Coord.first/avarage_window_size;
				AvarageOutputFirst.second += Coord.second/avarage_window_size;
			    channel_counter++;
			}
			return;
		};

		if(channel_counter == 2)
		{
			InputCoord.first = Coord.first - OutputSecondCenter.first; InputCoord.second = Coord.second - OutputSecondCenter.second; 

			AvarageOutputSecond.first += Coord.first/avarage_window_size;
			AvarageOutputSecond.second += Coord.second/avarage_window_size;
			accumulate_counter++;
			channel_counter--;
		}
    }
    // accumulate data to calc center positions
	if(WaitInputCoord == ZeroCoord && channel_counter == 1)
	{
		OutputFirstCenter.first += InputCoord.first/avarage_window_size;
		OutputFirstCenter.second += InputCoord.second/avarage_window_size;
		channel_counter++;
		qDebug() <<"FIRST CenTER: " << OutputFirstCenter.first <<OutputFirstCenter.second ;
		return;
	}

	if(WaitInputCoord == ZeroCoord && channel_counter == 2)
	{
		OutputSecondCenter.first += InputCoord.first/avarage_window_size;
		OutputSecondCenter.second += InputCoord.second/avarage_window_size;
		
		qDebug() <<"SECOND CenTER: " << OutputSecondCenter.first <<OutputSecondCenter.second ;
		accumulate_counter++; channel_counter--;
	}
    //==================================================================
	if(accumulate_counter == avarage_window_size) flag_filter_opencv = true;
}

void AccumulateDataFilter::WaitCoord(QPair<double,double> coord)
{
  //qDebug()<< TAG << "FILTER WAIT NEW COORD - " << coord.first << coord.second;
  WaitInputCoord = coord;	
  flag_filter_opencv = false;
  accumulate_counter = 0;
  pass_counter = 0;
}

AccumulateDataFilter& operator>>(QPair<double, double> coord, AccumulateDataFilter& Filter)
{
	Filter.SetInput(coord); return Filter;
}

std::pair<double,double> AccumulateDataFilter::GetFirstOutput()
{
	auto output_coord = std::make_pair(AvarageOutputFirst.first,AvarageOutputFirst.second);
	AvarageOutputFirst = QPair<double,double>(0,0);
  	return output_coord;
}

std::pair<double,double> AccumulateDataFilter::GetSecondOutput()
{
	flag_filter_opencv = false;  // close filter when two output coords is passed
	pass_counter++;
	auto output_coord = std::make_pair(AvarageOutputSecond.first,AvarageOutputSecond.second);
	AvarageOutputSecond = QPair<double,double>(0,0);
	return output_coord;
}

RotateDataMeasureengine& operator>>(std::pair<double,double> coord, RotateDataMeasureengine& Measureengine)
{
  Measureengine.CurrentMeasureCoordAbs.first = coord.first;
  Measureengine.CurrentMeasureCoordAbs.second = coord.second;
	return Measureengine;
}
//RotateDataMeasureengine& operator>>(QPair<double, double> coord, RotateDataMeasureengine& Measureengine)
//{
//  Measureengine.CurrentMeasureCoordAbs = coord; return Measureengine;
//}

void operator>>(RotateDataMeasureengine& Measureengine, RotateOperationContainer& RotateContainer)
{
	Measureengine.CurrentMeasureCoordAbs >> Measureengine.DataFilter >> RotateContainer;
	if(Measureengine.DataFilter.pass_counter > 0) Measureengine.SwitchToNextTestCoord();
}
	
void operator>>(AccumulateDataFilter& Filter, RotateOperationContainer& RotateContainer)
{
	if (Filter.flag_filter_opencv)
	{
		if(Filter.WaitInputCoord == QPair<double,double>(0,0))
		{
			Filter.pass_counter++; Filter.flag_filter_opencv = false;
			return;
		}; // when pass_counter > 0 measures switch to next coord

		RotateContainer.AppendInputData(Filter.GetFirstOutput());
		RotateContainer.AppendInputData(Filter.GetSecondOutput());
	}
}

void RotateDataMeasureengine::SwitchToNextTestCoord() { DataFilter.WaitCoord(DataVectors.GetTestCoord()); }

QPair<double, double> TestDataVectorsContainer::GetTestCoord()
{
		auto LastCoord = *CurrentTestCoord;
		CurrentTestCoord++; if (CurrentTestCoord == TestCoordVectorInput.end()) { CurrentTestCoord = TestCoordVectorInput.begin();};
		return LastCoord;
}
AccumulateDataFilter::AccumulateDataFilter() { }

void AccumulateDataFilter::reset()
{
	flag_filter_opencv = false;
	accumulate_counter = 0;
	pass_counter = 0;

	LastInputFirst = QPair<double,double>(0,0);
	WaitInputCoord = LastInputFirst;

	AvarageOutputFirst = LastInputFirst;
	AvarageOutputSecond = LastInputFirst;
	OutputFirstCenter = QPair<double,double>(0,0);
	OutputSecondCenter = QPair<double,double>(0,0);
}

RotateDataMeasureengine::RotateDataMeasureengine() 
{ 
	SwitchToNextTestCoord(); 
}
void RotateDataMeasureengine::Reset() 
{ 
	this->DataVectors.reset(); 
	SwitchToNextTestCoord(); 
}

void RotateOperationContainer::ConvertOperationToMatrix(pair<RotateAxis,torch::Tensor>& Operation, torch::Tensor& Matrix)
{
if(Operation.first == x_axis)
	Matrix = torch::sin(Operation.second) * x_sin + torch::cos(Operation.second) * x_cos + x_rotate_matrix_empty;
if(Operation.first == y_axis)
	Matrix = torch::sin(Operation.second) * y_sin + torch::cos(Operation.second) * y_cos + y_rotate_matrix_empty;
if(Operation.first == z_axis)
	Matrix = torch::sin(Operation.second) * z_sin + torch::cos(Operation.second) * z_cos + z_rotate_matrix_empty;
}



bool AccumulateDataFilter::CheckCoordMatch(QPair<double, double> Coord,QPair<double,double> AimCoord)
{
		double diff_x = Coord.first - AimCoord.first;
		double diff_y = Coord.second - AimCoord.second;
		double norm = std::hypot(diff_x, diff_y);
		//qDebug() << TAG<< "CHECK COORD IN FILTER - " << Coord << " aim coord - " << AimCoord << " norm - " << norm;
		if (norm < 1.2) return true;
		else return false;
}

void RotateOperationContainer::UpdateCommonMatrix()
{

    CommonMatrix = ones_matrix;
	RotateOperationMatrixSequense.clear();

	for(auto Operation : RotateOperationSequenseTensor)
	{
		RotateOperationMatrixSequense.push_back(ones_matrix);
		ConvertOperationToMatrix(Operation,RotateOperationMatrixSequense.back());
	    CommonMatrix = torch::matmul(RotateOperationMatrixSequense.back(), CommonMatrix);	
	}
	CommonMatrixInverse = torch::inverse(CommonMatrix);
	CommonSubMatrix = CommonMatrix.index({Slice(0,2),Slice(0,2)});
	CommonSubMatrixInverse = torch::inverse(CommonSubMatrix);
}
	
void RotateOperationContainer::AppendOperation(pair<RotateAxis,double> Operation)
{
    this->RotateOperationSequense.push_back(Operation);
    auto angle_radian = torch::tensor({Operation.second*3.14/180}, torch::TensorOptions().dtype(torch::kFloat32));
	auto operation_tensor = std::pair<RotateAxis,torch::Tensor>(Operation.first,angle_radian);
	this->RotateOperationSequenseTensor.push_back(operation_tensor);

	torch::Tensor matrix_operation; ConvertOperationToMatrix(operation_tensor,matrix_operation);
	CommonMatrix = torch::matmul(matrix_operation, CommonMatrix);
}

pair<torch::Tensor,torch::Tensor> RotateOperationContainer::ConvertInputCoord(vector<pair<double,double>> test_input ,vector<pair<double,double>> test_output, bool z_signs_inverse)
{
	int measure_count = test_input.size();
	vector<float> norms; norms.resize(measure_count); auto current_norm = norms.begin(); 

	int counter = 0;
	for(auto& coord: test_output)
	{
	   *current_norm = pow(coord.first,2) + pow(coord.second,2);
	   current_norm++;
	} // X2 + Y2 from rotated vector
	vector<float>::iterator max_element_first = max_element(norms.begin(), norms.end() - norms.size()/2);            //X2+Y2 first peak
	vector<float>::iterator min_element_first = min_element(max_element_first, max_element_first + norms.size()/4);  //X2+Y2 min
	vector<float>::iterator max_element_second = max_element(min_element_first, min_element_first + norms.size()/4); //second peak

	int pos_max_norm1 = distance(norms.begin(),max_element_first);  //first peak X2+Y2 POS
	int pos_max_norm2 = distance(norms.begin(),max_element_second);  //second peak X2+Y2 POS

    //==========================================================
    measure_count = test_input.size();
	torch::Tensor input_matrix = torch::ones({3,measure_count});
	torch::Tensor output_matrix = torch::ones({3,measure_count});
	qDebug()<< TAG << "MEASURE COUNT : " << measure_count;


           for(int n = 0; n < measure_count; n++)
           {
               float vector[] = {float(test_input[n].first), float(test_input[n].second), 0};
               input_matrix.index({Slice(None),Slice(n,n+1)}) = torch::from_blob(vector,{3,1});
           
               //calc vector z component manually because from camera gets only x y  
               float norm = *torch::norm(input_matrix.index({Slice(None),Slice(n,n+1)}),2,0).data_ptr<float>(); 
               double z_component =  std::sqrt(std::pow(norm,2) - std::pow(test_output[n].first,2) - std::pow(test_output[n].second,2)); 
               float vector_output[3] = {float(test_output[n].first), float(test_output[n].second), float(z_component)};
               output_matrix.index({Slice(None),Slice(n,n+1)}) = torch::from_blob(vector_output,{3,1});
           }
           output_matrix = torch::nan_to_num(output_matrix, 0);

	for(int n = 0; n < measure_count; n++)
	{
		if(n < pos_max_norm1 || n > pos_max_norm2)
		output_matrix.index({Slice(2), Slice(n,n+1)}) = output_matrix.index({Slice(2), Slice(n,n+1)})*-1;
	} 

	if(z_signs_inverse) output_matrix.index({Slice(2), Slice(None)}) = output_matrix.index({Slice(2), Slice(None)})*-1;
    //==========================================================
	return std::make_pair(input_matrix,output_matrix);
}

torch::Tensor RotateOperationContainer::FitToTestVectors(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output, float gamma, float speed, int number_iteration)
{
	auto test_data = ConvertInputCoord(test_input,test_output,false);
	return FitToTestVectors(test_data.first,test_data.second,gamma,speed, number_iteration);
}

torch::Tensor RotateOperationContainer::FitToTestVectors(torch::Tensor test_input, torch::Tensor test_output, float gamma, float speed, int number_iteration)
{
      torch::Tensor loss;
      vector<float> momentums; momentums.resize(3); auto current_momentum = momentums.begin();
			momentums[0] = 0; momentums[1] = 0; momentums[2] = 0;

			for (int n = 0; n <= number_iteration; n++)
			{

			 for(auto& rotate_operation: RotateOperationSequenseTensor)
			 rotate_operation.second.set_requires_grad(true);

			 UpdateCommonMatrix();

			auto output = torch::matmul(CommonMatrix, test_input);
			auto error_output = output - test_output;
			auto distance = torch::norm(error_output,2,0);
			loss = torch::sum(torch::pow(distance, 2))/float(distance.numel());
			loss.backward();


			 for(auto& angle_rotate: RotateOperationSequenseTensor)
			 angle_rotate.second.set_requires_grad(false);

			 for(auto& rotate_operation: RotateOperationSequenseTensor)
			 {
			 *current_momentum = *current_momentum*gamma + speed*rotate_operation.second.grad().data_ptr<float>()[0]; 
			 rotate_operation.second = rotate_operation.second - *current_momentum; 
			 current_momentum++; if(current_momentum == momentums.end()) {current_momentum =  momentums.begin();};

			 }

			 if (n % 500 == 0)
			 {
			 auto Angle = *RotateOperationSequenseTensor[0].second[0].data_ptr<float>() * 180 / 3.14;
			 qDebug()  << "   LOSS - " << fixed << qSetRealNumberPrecision(3) << loss.data_ptr<float>()[0] << " | "<< Angle;
			 }

			 if(loss.data_ptr<float>()[0] < 0.00005) break;
			}

			 for(int n = 0; n < RotateOperationSequense.size(); n++)
				 RotateOperationSequense[n].second = *RotateOperationSequenseTensor[n].second[0].data_ptr<float>() * 180/3.14;

			
			return loss;
}

double RotateOperationContainer::CalcTransformationScale(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output)
{
	vector<double> test_output_norm_massive; test_output_norm_massive.resize(test_input.size());
	double input_norm = std::hypot(test_input[0].first,test_input[0].second);

	for(int number = 0; number < test_output_norm_massive.size(); number++)
	{
		test_output_norm_massive[number] = pow(test_output[number].first,2) + pow(test_output[number].second,2);
	}
	auto max_value_norm = *std::max_element(test_output_norm_massive.begin(),test_output_norm_massive.end());
	return std::sqrt(max_value_norm/(input_norm*input_norm));
}

void RotateOperationContainer::FindArbitraryRotationToVectors(vector<pair<double,double>> test_input, vector<pair<double,double>> test_output)
{
	qDebug() << TAG << "FIND ARBITRARY ROTATION TO VECTORS";
	input_to_optimize_rotation = test_input;
	output_to_optimize_rotation = test_output;


		pixel_scale_remote_to_local = CalcTransformationScale(test_input, test_output);
		this->system_transform_scale = pixel_scale_remote_to_local;
		qDebug() << TAG << "        SYSTEM SCALE " << pixel_scale_remote_to_local;

		for (auto& Coord : test_output) //SCALE VECTORS TO BASE SYSTEM
		{
			Coord.first  /= pixel_scale_remote_to_local;
			Coord.second /= pixel_scale_remote_to_local;
		}

    auto FitTestVectorsToRotation = [this](pair<torch::Tensor,torch::Tensor>  TestVectors)
	{
       vector<vector<RotateAxis>> Sequenses;
								  Sequenses.push_back({z_axis,x_axis,y_axis}); 
								  Sequenses.push_back({z_axis,y_axis,x_axis}); 
								  Sequenses.push_back({y_axis,x_axis,z_axis}); 
								  Sequenses.push_back({y_axis,z_axis,x_axis}); 
								  Sequenses.push_back({x_axis,y_axis,z_axis}); 
								  Sequenses.push_back({x_axis,z_axis,y_axis}); 
	    vector<RotateOperationContainer> RotateVariants;

		vector<float> Losses;
		for(auto Sequence: Sequenses)
		{
		float speed = 0.000003;
		float momentum = 0.9;
		float number_iteration = 4000;
		RotateOperationContainer Rotate;
								Rotate.AppendOperation(pair<RotateAxis,double>(Sequence.at(0),52));
								Rotate.AppendOperation(pair<RotateAxis,double>(Sequence.at(1),40));
								Rotate.AppendOperation(pair<RotateAxis,double>(Sequence.at(2),0));

			RotateVariants.push_back(Rotate);
			qDebug() << "CHECK - " << RotateVariants.back().RotationToString().c_str();
			auto Loss = RotateVariants.back().FitToTestVectors(TestVectors.first,TestVectors.second,momentum,speed, number_iteration);

			Losses.push_back(*Loss.data_ptr<float>());
			RotateVariants.back().system_transform_scale = pixel_scale_remote_to_local;
			RotateVariants.back().pixel_scale_remote_to_local = pixel_scale_remote_to_local;
		}

		for(int n = 0; n<Losses.size(); n++)
			qDebug() << QString(RotateVariants[n].RotationToString().c_str()) << " [ enD LOSS - " << Losses[n] << " ]";

		auto min = min_element(Losses.begin(),Losses.end());
		int number_min_loss = distance(Losses.begin(),min);

		this->CopyRotation(RotateVariants[number_min_loss]);
		qDebug() << TAG << "OPT MATRIX BEST ROTATION";
		qDebug() << TAG << QString::fromStdString(RotationToString());
		qDebug() << TAG << QString::fromStdString(MatrixToString());
		qDebug() << TAG << "        SYSTEM SCALE " << RotateVariants[number_min_loss].pixel_scale_remote_to_local;
	};

	auto test_data_z = ConvertInputCoord(test_input,test_output,false);
	auto test_data_inverse_z = ConvertInputCoord(test_input,test_output,true);
   
    FitTestVectorsToRotation(test_data_z);
	FitTestVectorsToRotation(test_data_inverse_z);
	
	SaveMeasureDataToFile(QString("/home/spp/DATA/TrainerData/MEASURES/MeasureData.txt"));
	qDebug() << "=================================================================================" << Qt::endl;

	RotationValid = true; this->Inverse();
}


torch::Tensor RotateOperationContainer::ConvertInput(torch::Tensor& Input)
{
torch::Tensor output;
if(!is_rotation_inverse) output = torch::matmul(CommonMatrix, Input);	
if(is_rotation_inverse)  output = torch::matmul(CommonMatrixInverse, Input);	
return output;
}

RotateOperationContainer::RotateOperationContainer(const RotateOperationContainer& CopyContainer) : RotateOperationContainer()
{
	this->CopyRotation(CopyContainer);
}

void RotateOperationContainer::CopyRotation(const RotateOperationContainer& CopyContainer)
{
	CommonMatrix = CopyContainer.CommonMatrix;
	CommonMatrixInverse = CopyContainer.CommonMatrixInverse;
	CommonSubMatrix = CopyContainer.CommonSubMatrix;
	CommonSubMatrixInverse = CopyContainer.CommonSubMatrixInverse;
	
	RotateOperationSequense = CopyContainer.RotateOperationSequense;
	RotateOperationMatrixSequense = CopyContainer.RotateOperationMatrixSequense;
	RotateOperationSequenseTensor = CopyContainer.RotateOperationSequenseTensor;
	OutputCoord = CopyContainer.OutputCoord;
	InputCoord = CopyContainer.InputCoord;

	pixel_scale_remote_to_local = CopyContainer.pixel_scale_remote_to_local;
	system_transform_scale = CopyContainer.system_transform_scale;
	is_rotation_inverse = CopyContainer.is_rotation_inverse;
	//=====================================================
}

RotateOperationContainer::RotateOperationContainer() { Reset(); }

void RotateOperationContainer::Reset()
{
	input_to_optimize_rotation.clear();
	output_to_optimize_rotation.clear();
	MeasureFilter.Reset();

	counter_test = 0; // to delete
	RotationValid = false;
	system_transform_scale = 1;
	pixel_scale_remote_to_local = 1;

     ones_matrix = torch::tensor({ {1,0,0},{0,1,0},{0,0,1} }, torch::TensorOptions().dtype(torch::kFloat32));
     CommonMatrix = ones_matrix;;
     x_sin = torch::tensor({ {0,0,0},{0,0,-1},{0,1,0} }, torch::TensorOptions().dtype(torch::kFloat32));
     x_cos = torch::tensor({ {0,0,0},{0,1, 0},{0,0,1} }, torch::TensorOptions().dtype(torch::kFloat32));

     y_sin = torch::tensor({ {0,0,1},{0,0,0},{-1,0,0} }, torch::TensorOptions().dtype(torch::kFloat32));
     y_cos = torch::tensor({ {1,0,0},{0,0, 0},{0,0,1} }, torch::TensorOptions().dtype(torch::kFloat32));

     z_sin = torch::tensor({ {0,-1,0},{1,0,0},{0,0,0} }, torch::TensorOptions().dtype(torch::kFloat32));
     z_cos = torch::tensor({ {1, 0,0},{0,1,0},{0,0,0} }, torch::TensorOptions().dtype(torch::kFloat32));

     x_rotate_matrix_empty = torch::tensor({ {1,0,0},{0,0,0},{0,0,0} }, torch::TensorOptions().dtype(torch::kFloat32));
     y_rotate_matrix_empty = torch::tensor({ {0,0,0},{0,1,0},{0,0,0} }, torch::TensorOptions().dtype(torch::kFloat32));
     z_rotate_matrix_empty = torch::tensor({ {0,0,0},{0,0,0},{0,0,1} }, torch::TensorOptions().dtype(torch::kFloat32));

	 InputCoord = torch::zeros({3,1},torch::TensorOptions().dtype(torch::kFloat32));
	 OutputCoord = torch::zeros({3,1},torch::TensorOptions().dtype(torch::kFloat32));
}

string RotateOperationContainer::MatrixToString()
{
    std::ostringstream out; out.precision(5); out << this->CommonMatrix;
	return out.str();
}

string RotateOperationContainer::RotationToString(vector<pair<RotateAxis,double>> Sequence)
{
    std::ostringstream out; out.precision(4);
	for(auto Rotation: Sequence)
	{
	  if(Rotation.first == x_axis)	out << " [ x_axis ] ";
	  if(Rotation.first == y_axis)	out << " [ y_axis ] ";
	  if(Rotation.first == z_axis)	out << " [ z_axis ] ";
	  out <<std::setw(5)<< Rotation.second;
	}
	return out.str();
}

string RotateOperationContainer::RotationToString()
{
	return RotationToString(RotateOperationSequense);
}

void RotateOperationContainer::AppendInputData(pair<double,double> test_coord)
{
   if (input_to_optimize_rotation.size() == output_to_optimize_rotation.size())
    input_to_optimize_rotation.push_back(std::make_pair(test_coord.first,test_coord.second));
   else
   {
	output_to_optimize_rotation.push_back(test_coord);
	auto& coord = input_to_optimize_rotation.back();
	auto& coord2 = output_to_optimize_rotation.back();
	qDebug() << "APPEND " << coord.first << coord.second << " | " << coord2.first << coord2.second << " COUNT: " << counter_test++;
   }
   
   if(IsDataFull()) FindArbitraryRotationToVectors(input_to_optimize_rotation, output_to_optimize_rotation);
}


void RotateOperationContainer::SetInput(const QPair<double,double>& Coord)
{
		 InputCoord[0] = Coord.first;
		 InputCoord[1] = Coord.second;
		 InputCoord[2] = 0;

		 if(is_rotation_inverse)
		 InputCoord[2] = CalcVirtualZComponent(Coord);
                               
         OutputCoord = this->ConvertInput(InputCoord);
		 OutputCoordVector[0] = OutputCoord.data_ptr<float>()[0];
		 OutputCoordVector[1] = OutputCoord.data_ptr<float>()[1];
		 OutputCoordVector[2] = OutputCoord.data_ptr<float>()[2];

		 if(!is_rotation_inverse) OutputCoord = OutputCoord*system_transform_scale;
		 if( is_rotation_inverse) OutputCoord = OutputCoord/system_transform_scale;
}

const QPair<double, double>& RotateOperationContainer::GetOutput()
{
	return QPair<double, double>(this->OutputCoord.data_ptr<float>()[0], this->OutputCoord.data_ptr<float>()[1]);
}

std::vector<float> RotateOperationContainer::GetOutputVector() 
{
    return OutputCoordVector;
}


void RotateOperationContainer::Inverse()
{
	//flag needed only to calculate z_component when used inverse rotation
	is_rotation_inverse = !is_rotation_inverse;
}

double RotateOperationContainer::CalcVirtualZComponent(QPair<double,double> InputCoord)
{

	torch::Tensor InputCoordTensor = torch::tensor({InputCoord.first,InputCoord.second}, torch::TensorOptions().dtype(torch::kFloat32)).t();
	torch::Tensor OutputCoordTensor = torch::tensor({0,0,0}, torch::TensorOptions().dtype(torch::kFloat32)).t();

	OutputCoordTensor.index({Slice(0,2)}) = torch::matmul(CommonSubMatrixInverse,InputCoordTensor);

	OutputCoordTensor = torch::matmul(CommonMatrix,OutputCoordTensor);
	auto VectorValues = OutputCoordTensor.data_ptr<float>();

	//qDebug() << "Input coord inverse - " << VectorValues[0] << VectorValues[1] << VectorValues[2];
	return OutputCoordTensor.data_ptr<float>()[2];
}

void RotateOperationContainer::SetRotateMatrix(torch::Tensor RotateMatrix)
{
	   CommonMatrix = RotateMatrix;
	   CommonMatrixInverse = torch::inverse(CommonMatrix);
	   CommonSubMatrix = CommonMatrix.index({Slice(0,2),Slice(0,2)});
	   CommonSubMatrixInverse = torch::inverse(CommonSubMatrix);
}

void RotateOperationContainer::SaveMeasureDataToFile(QString Filename)
{
    qDebug() << "Save data to file - " << Filename ;
    QFile data(Filename);
    data.open(QFile::WriteOnly); data.flush();

	QString outString;
	QTextStream out(&outString);

	out << "MATRIX" << Qt::endl;
	out << this->MatrixToString().c_str() << Qt::endl;
	out <<"===========================================================" << Qt::endl;
	out <<"ROTATION" << Qt::endl;
	out << this->RotationToString().c_str() << Qt::endl;
	out <<"===========================================================" << Qt::endl;
	out << "SCALE            " << pixel_scale_remote_to_local << Qt::endl;
	out <<"===========================================================" << Qt::endl;
	out << "MEASURE_NUMBER   " << input_to_optimize_rotation.size() << Qt::endl;
	out <<"===========================================================" << Qt::endl;

	out << qSetFieldWidth(20) << Qt::right << "Base point" << "Rotate point" << "" << Qt::endl;
	out << qSetFieldWidth(10) << Qt::right << " x " << " y " << " x " << " y " << Qt::endl;

	for(int n = 0; n < input_to_optimize_rotation.size(); n++)
	{
		out << input_to_optimize_rotation[n].first << input_to_optimize_rotation[n].second
			<< output_to_optimize_rotation[n].first << output_to_optimize_rotation[n].second << Qt::endl;
	}
	data.write(outString.toUtf8());
	data.close();
}

void RotateOperationContainer::LoadRotationFromFile(QString Filename)
{

     QFile data(Filename);
	 data.open(QFile::ReadOnly); 

	 if(!data.isOpen()){ qDebug() << TAG << " FILE NOT opencv: " << Filename;  return; }

     QTextStream in_stream(&data);
	 QString header = in_stream.readLine();

	 rotate_matrix[9] = {0};

	 for(auto& value: rotate_matrix)
		 in_stream >> value; 
	 in_stream.readLine(); in_stream.readLine(); in_stream.readLine(); in_stream.readLine();
     //=================================================================
	 std::vector<pair<RotateAxis,double>> rotate_operation; 
	 QString axis_string;  RotateAxis axis = x_axis; double angle_degree_value = 0;

	 for(int n = 1; n <=3; n++)
	 {
	 in_stream >> axis_string; in_stream >> angle_degree_value;
	 axis = x_axis; if(axis_string == "y_axis"){axis = y_axis;}; if(axis_string == "z_axis"){axis = z_axis;}; 
	 rotate_operation.push_back(make_pair(axis,angle_degree_value));
	 }
     //=================================================================
	 int number_measure = 0;
	 in_stream.readLine(); in_stream.readLine();

	 in_stream >> header >> this->system_transform_scale; pixel_scale_remote_to_local = system_transform_scale;
	 in_stream.readLine(); in_stream.readLine();
	 in_stream >> header; in_stream >> number_measure;
	 in_stream.readLine(); in_stream.readLine(); in_stream.readLine(); in_stream.readLine();

	 std::vector<pair<double,double>> input_measures; input_to_optimize_rotation.resize(number_measure);
	 std::vector<pair<double,double>> output_measures; output_to_optimize_rotation.resize(number_measure);

	 for(int n=0; n < number_measure; n++)
	 {
        in_stream >> input_to_optimize_rotation[n].first; in_stream >> input_to_optimize_rotation[n].second;
        in_stream >> output_to_optimize_rotation[n].first; in_stream >> output_to_optimize_rotation[n].second;

        output_to_optimize_rotation[n].first = output_to_optimize_rotation[n].first; 
        output_to_optimize_rotation[n].second = output_to_optimize_rotation[n].second; 
	 }

	 CommonMatrix = torch::from_blob(rotate_matrix,{3,3}); 
	 CommonMatrixInverse = torch::inverse(CommonMatrix);
	 CommonSubMatrix = CommonMatrix.index({Slice(0,2),Slice(0,2)});
	 CommonSubMatrixInverse = torch::inverse(CommonSubMatrix);
	 RotationValid = true;

     //qDebug() << TAG << "SCALE: " << system_transform_scale << " ROTATION: " << RotationToString(rotate_operation).c_str();
	 //qDebug() << TAG << "MATRIX   :" << MatrixToString().c_str();
	 //qDebug() << TAG << Filename;
	 //qDebug() << "=============================" << endl;
	 //qDebug()<< TAG << "MEASURES :" << number_measure;
}

bool RotateOperationContainer::isValid() { return RotationValid; }

//void RotateOperationContainer::operator=(RotateOperationContainer & Rotation) //{ //	CommonMatrixInverse = torch::inverse(CommonMatrix); //}
//void RotateOperationContainer::operator=(RotateOperationContainer && Rotation) //{ //}

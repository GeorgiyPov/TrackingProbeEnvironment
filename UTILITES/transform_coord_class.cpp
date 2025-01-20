#include "transform_coord_class.h"
#include <math.h>


const QPair<double, double>& TransformCoordClass::GetOutput() { return TransformedCoord; }
void TransformCoordClass::SetInput(const QPair<double,double>& Coord) { TransformCoord(Coord); }

TransformCoordClass::TransformCoordClass()
{
    Scale = 3.14 / (60.0 * 60.0 * 180.0);
	Offset = 0;
	TransformCoord = [this](QPair<double,double> CoordError)
	{
		TransformedCoord.first = CoordError.first * Scale + Offset;   if(TransformedCoord.first > SaturationFirst) TransformedCoord.first = SaturationFirst;
		TransformedCoord.second = CoordError.second * Scale + Offset; if(TransformedCoord.second > SaturationSecond) TransformedCoord.second = SaturationSecond;

	};
	//TRANSFORM SECS TO RADIAN
}
TransformCoordClass::TransformCoordClass(double ScaleParam, double OffsetParam, double Saturation1, double Saturation2)
{
	Scale = ScaleParam;
	Offset = OffsetParam;
	TransformCoord = [this](QPair<double,double> CoordError)
	{
		TransformedCoord.first = CoordError.first * Scale  + Offset;  if(TransformedCoord.first > SaturationFirst) TransformedCoord.first = SaturationFirst;
		TransformedCoord.second = CoordError.second * Scale + Offset; if(TransformedCoord.second > SaturationSecond) TransformedCoord.second = SaturationSecond;

	};
}

TransformCoordClass::TransformCoordClass(const TransformCoordClass& CopyTransform)
{
	Scale = CopyTransform.Scale;
	TransformCoord = CopyTransform.TransformCoord;
}


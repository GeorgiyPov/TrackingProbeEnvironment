#include "engine_statistics.h"
#include "interface_pass_coord.h"
static const char* TAG = "[ STATISTIC ]";


int StatisticGroup::GetBestStatisticsCoord()
{
    return FindBestStatisticCoord(this->BeginStatistic,this->endStatistic);
};
int StatisticGroup::GetBestStatisticsValue()
{
   return FindBestStatisticValue(this->BeginStatistic, this->endStatistic);
};

double Statistic::Norm(QPair<double,double> Coord)
{
  return std::sqrt(std::pow(Coord.first,2) + std::pow(Coord.second,2));
}


void operator>>(double NewValue, StatisticGroup& StatObj)
{
    Statistic& CurrentStat = StatObj.CurrentStatistic->second;
    Statistic& BeginStat = StatObj.CurrentStatistic->second;
    Statistic& endStat = StatObj.Statistics[StatObj.Statistics.size()];

    if (endStat.IsValueLoaded()) return;
        NewValue >> CurrentStat;

        if(CurrentStat.IsValueLoaded()) StatObj.CurrentStatistic++;
            if(endStat.IsValueLoaded()) StatObj.BestStatNumberValue = StatObj.GetBestStatisticsValue();

};

void operator>>(QPair<double, double> NewValue, StatisticGroup& StatObj)
{
if (StatObj.IsStatisticsLoaded()) return;

NewValue >> StatObj.CurrentStatistic->second;

if(StatObj.CurrentStatistic->second.IsCoordLoaded()) { StatObj.CurrentStatistic++; StatObj.BestStatNumber++; };

if(StatObj.IsStatisticsLoaded()) StatObj.BestStatNumber = StatObj.GetBestStatisticsCoord(); ;

};


StatisticGroup::StatisticGroup(int Count, int WindowSize)
{
    for(int n = 0; n < Count; n++) Statistics.insert(std::make_pair(n,Statistic(WindowSize)));

    BeginStatistic = Statistics.begin();
    CurrentStatistic = BeginStatistic;
    endStatistic = Statistics.end();


    FindBestStatisticCoord = [](std::map<int,Statistic>::iterator BeginStatistic,std::map<int,Statistic>::iterator endStatistic) -> int
    {
        auto current_stat = BeginStatistic;
        std::vector<float> dispersions;

        while(current_stat != endStatistic) {dispersions.push_back(current_stat->second.DispersionCoordDistance); current_stat++;};

        auto min_element = std::min_element(dispersions.begin(), dispersions.end());
        auto min_pos = std::distance(dispersions.begin(),min_element);
        qDebug() << TAG << "BEST THRESHOLD RANGE FOUND " << 0.6 + float(min_pos)/100 << " POSITION - " << min_pos;

        return min_pos;
    };

    FindBestStatisticValue = [](std::map<int,Statistic>::iterator BeginStatistic,std::map<int,Statistic>::iterator endStatistic) -> int
    {
            auto current_stat = BeginStatistic;
            std::vector<float> dispersions;

            while (current_stat != endStatistic) { dispersions.push_back(current_stat->second.DispersionValue); current_stat++;};

            auto min_element = std::min_element(dispersions.begin(), dispersions.end());
            auto min_pos = std::distance(dispersions.begin(), min_element);

        return min_pos;
    };
};

void StatisticGroup::Reset()
{
    for(auto& Stat: Statistics)  Stat.second.Reset(); BestStatNumber = 1;

    BeginStatistic = Statistics.begin();
    CurrentStatistic = BeginStatistic;
    endStatistic = Statistics.end();

}

StatisticGroup::StatisticGroup(const StatisticGroup& Group)
{
Statistics = Group.Statistics;
BeginStatistic = Statistics.begin();
CurrentStatistic = BeginStatistic;
endStatistic = Statistics.end();
};

void StatisticGroup::operator=(const StatisticGroup& Group)
{
Statistics = Group.Statistics;
BeginStatistic = Statistics.begin();
CurrentStatistic = BeginStatistic;
endStatistic = Statistics.end();
}
void StatisticGroup::PerformAvailableData()
{
    if(Statistics.size() < 2) { BestStatNumber = 0; return;};

    Statistics.erase(--CurrentStatistic,endStatistic); endStatistic = Statistics.end(); //DELETE NON FILLED RECORDS

    BestStatNumber = GetBestStatisticsCoord();
};

void Statistic::CalcDispersion()
{
		DispersionCoord = QPair<double, double>(0, 0);
		DispersionValue = 0;

            QPair<double, double> CoordDeviation;
            std::vector<double> DeviationMeasures;
			for (auto Coord : CoordMassive)
			{

				DispersionCoord.first += std::pow(Coord.first - AvarageCoord.first, 2) / CoordMassive.size();
				DispersionCoord.second += std::pow(Coord.second - AvarageCoord.second, 2) / CoordMassive.size();
				CoordDeviation = (Coord - AvarageCoord);
                DeviationMeasures.push_back(std::hypot(CoordDeviation.first,CoordDeviation.second));
			}
            DispersionCoordDistance = std::hypot(DispersionCoord.first,DispersionCoord.second);
			AmplitudeCoordDeviation = *(std::max_element(DeviationMeasures.begin(),DeviationMeasures.end()));


			for(auto Value: ValueMassive)
			DispersionValue += std::pow(Value - AvarageValue, 2) / ValueMassive.size();

}


void Statistic::Reset()
{
	this->CoordMassive.clear();
	this->ValueMassive.clear();
	this->AvarageCoord = QPair<double, double>(0, 0);
	this->DispersionCoord = QPair<double,double>(0,0);
	this->MinValue = 0; this->MaxValue = 0; this->AvarageValue = 0;
	this->Counter = 0;
    this->DispersionCoordDistance = 0;
}


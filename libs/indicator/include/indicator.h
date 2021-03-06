#ifndef INDICATOR_H
#define INDICATOR_H

#include "common_def.h"

#ifdef INDICATOR_STATICLIB
#define INDICATOR_API
#else
#ifdef INDICATOR_EXPORTS
#define INDICATOR_API SHARED_DECL_EXPORT
#else
#define INDICATOR_API SHARED_DECL_IMPORT
#endif
#endif

#include <vector>

struct INDICATOR_API MovingAveragePoint
{
	double x;

	double movingAverage;

	double movingDispersion;
};

/**	@brief Класс технического индикатора изменения курса валюты */
class INDICATOR_API CIndicator
{

public:

/**	@name Конструктор и деструктор
	@{ */
	CIndicator(std::vector< std::pair< double, double > > timeSamples);

	~CIndicator();
/**	@} */

/**	@brief Оператор присваивания */
	CIndicator & operator = (const CIndicator & other);

/**	@brief Получение значений скользящей регрессии в каждой точке исходной зависимости */
	std::vector< std::pair< double, double > > GetMovingRegression(int winWidth, int polyDegree);

/**	@brief Получение значений скользящего среднего в каждой точке исходной зависимости */
	std::vector< std::pair< double, double > > GetMovingAverage(int winWidth, int step);

	std::vector< MovingAveragePoint > GetRealTimeMovingRegression(int winWidth, int step);

//	std::vector< std::pair< double, double > > GetRealTimeMovingAverage(int winWidth, int step);

	std::vector< MovingAveragePoint > GetRealTimeMovingAverage(int winWidth, int step);
	

private:

	double GetSingleMovingRegression(int winWidth, int polyDegree, int index);

	double GetSingleMovingAverage(int winWidth, int index);

/**
@return выборочное_среднее, выборочная_дисперсия
*/
	std::pair< double, double > GetSingleRealTimeMovingAverage(int winWidth, int index);
	std::pair< double, double > GetSingleRealTimeMovingRegression(int winWidth, int polyDegree, int index);

	std::vector< std::pair< double, double > > m_timeSamples; //!< исходная зависимость величины (цены) от времени

	

};

#endif // INDICATOR_H
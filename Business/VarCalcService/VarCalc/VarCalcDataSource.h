#ifndef _vardatasource_h_1987192
#define _vardatasource_h_1987192

#include "../../../../Core/TxMath/ArrayMatrix.h"

namespace Tx
{
	namespace VAR
	{
		class CVarDataSource
		{
		public:
			CVarDataSource();
			virtual ~CVarDataSource();

		public:
			void Clear();
			bool AssertValid();

			bool FillData(Tx::Core::CArrayMatrix& matrixPrice, Tx::Core::CArrayMatrix& matrixStorage);

			int GetSamplesCount();
			int GetSampleDays();

			void GetPricesMatrix(Tx::Core::CArrayMatrix& matrixPrice);
			void GetStorageMatrix(Tx::Core::CArrayMatrix& matrixStorage);
			void GetLatestValeMatrix(Tx::Core::CArrayMatrix& matrixLatestValue);
			bool GetPLRateMatrix(int iType, int iSpan, Tx::Core::CArrayMatrix& matrixPLRate);
		protected:
			bool TransformCoeff();
		protected:
			Tx::Core::CArrayMatrix m_matrixPrice;// m_iDateCount * m_iSamplesCount   维行优先矩阵。
			Tx::Core::CArrayMatrix m_matrixStorage; //m_iSamplesCount * 1   维向量
			Tx::Core::CArrayMatrix m_matrixLatestValue;//m_iSamplesCount * 1   维向量

			int m_iSamplesCount;
			int m_iDateCount;
		};
	}
}

#endif
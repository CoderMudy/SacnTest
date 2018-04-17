#ifndef _functions_h_1987192
#define _functions_h_1987192

void gnNrmlArr(unsigned seed, double * Arr, int length);
double sclprd(double * a, double * b, int n);
void gen_norm_vect(double * cov,int n,int m,double * result);
int covar(double * x, int n, int m, double lmd, 
		  double * cov,
		  double * avg  
		  );
int covar2(double * x, int n, int m, double lmd, 
		  double * cov,
		  double * avg  
		  );
double var_of_comb(double * coeff,double * cov, int n);
double percentile(double alpha);
int normal_method(double * sample,
				  int n, int m,
				  double lmd,
				  double * coeff,
				  int T,
				  double alpha,
				  double * var, 
				  double * cvar,
				  double * avg
				  );
void percentile_from_dencity(double w[][2], 
							 int n, double alpha, 
							 double * VAR, double * CVAR);

void precac(int type,
			int m, int n,
			double * y,
			int span 
			);

void transform_coeff(double * coeff,
					 int m,
					 int n,
					 double * sample
					 );

void history_method(
					double * sample,
					int n, int m,
					double lmd,
					double * coeff,
					int T, 
					double alpha,
					double * var, 
					double * cvar
				  );

void monte_carlo_method(
					double * sample,
					int n, int m,
					int M,
					double lmd,
					double * coeff,
					int T,
					double alpha,
					double * var,
					double * cvar
					);

void caculate_var(
		double * sample,//n个头寸收益率,m天样本矩阵，m行n列
					//第0天是最远的一天
		int n, int m,
		int M,//做M次随机模拟
		double lmd,//公比，按天等比衰减权重
		double * coeff,//n个头寸持有金额，向量
		int T, //未来持有天数
		double alpha,//置信度,<0.5,比如0.05
		double * var, //返回值
		double * cvar,//返回值
		int method,//0代表正态法，1代表历史模拟法,2代表蒙特卡罗
		int type//0-收益，1-百分比收益率，2-对数收益率
		);
#endif
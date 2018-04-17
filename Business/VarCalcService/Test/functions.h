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
		double * sample,//n��ͷ��������,m����������m��n��
					//��0������Զ��һ��
		int n, int m,
		int M,//��M�����ģ��
		double lmd,//���ȣ�����ȱ�˥��Ȩ��
		double * coeff,//n��ͷ����н�����
		int T, //δ����������
		double alpha,//���Ŷ�,<0.5,����0.05
		double * var, //����ֵ
		double * cvar,//����ֵ
		int method,//0������̬����1������ʷģ�ⷨ,2�������ؿ���
		int type//0-���棬1-�ٷֱ������ʣ�2-����������
		);
#endif
#include "stdafx.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

//���ɷ�����̬�ֲ����������
void gnNrmlArr(unsigned seed, double * Arr, int length)
{

	int i,j;
	srand(seed);
	for(i=0;i<length;i++)
	{
	   *(Arr+i)=0;
	   for(j=0;j<6;j++)
	   {
		   *(Arr+i) += (rand()-rand())/32768.0;
	   }
	}
}
//�������
double sclprd(double * a, double * b, int n)
{

	int i;
	double result=0;
	for(i=0;i<n;i++)
	{
		result += a[i]*b[i];
	}
	return result;
}

void gen_norm_vect(double * cov,int n,int m,double * result)
{
	int i,j,k;
	double * a= new double [n*n];
	double temp1,temp2;
	double * rnd=new double [n*m];
	
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			a[i*n+j]=0;
		}
	}
	for(i=0;i<n;i++)
	{
		for(j=0;j<=i;j++)
		{
			temp1=0;
			temp2=0;
			for(k=0;k<j;k++)
			{
				temp1+=a[i*n+k]*a[j*n+k];
				temp2+=a[j*n+k]*a[j*n+k];
			}
			a[i*n+j]=cov[i*n+j]-temp1;
			a[i*n+j]/=sqrt(cov[j*n+j]-temp2);
		}
	}


	gnNrmlArr((unsigned)time( NULL ),  rnd, n*m); 
	for(k=0;k<m;k++)
	{
		for(j=0;j<n;j++) 
		{			
			result[k*n+j]=sclprd(a+n*j, rnd+k*n, n);
		}
	}

	delete [] a;
	delete [] rnd;
}

//����Э�����������
int covar(double * x, int n, int m, double lmd, 
		  double * cov, 
		  double * avg  
		  )
{

	if(lmd>1||lmd<=0) return -1;
	int i,j,k,h;
	double * LMD=new double[m]; 
	double temp=1,sum=0;

	//LMD[i] = (1-lmd)*pow(lmd, m - i) /(1 - pow(lmd, m))
	for(i=m-1;i>=0;i--)
	{
		LMD[i]= temp;
		sum+=temp; 
		temp*=lmd; 
	}
	for(i=0;i<m;i++) LMD[i]/=sum;

	for(i=0;i<n;i++)
	{
		sum=0;
		for(j=0;j<m;j++)
		{
			sum += LMD[j] * x[n*j+i];
		}
		avg[i]=sum;
	}

	for(i=0;i<n;i++)
	{
		for(j=0;j<=i;j++)
		{
			h=i*n+j;
			cov[h]=0;
			for(k=0;k<m;k++)
			{
				cov[h]+=LMD[k]*x[k*n+i]*x[k*n+j];
				//cov[h]+=LMD[k]* (x[k*n+i] - avg[i]) * (x[k*n+j] - avg[j]);

			}
			cov[h]-=avg[i]*avg[j];
			cov[j*n+i]=cov[h]; 
		}
	}

	delete[] LMD;
	return 0;
}

//����Э�����������
int covar2(double * x, int n, int m, double lmd, 
		  double * cov, 
		  double * avg  
		  )
{

	if(lmd>1||lmd<=0) return -1;
	int i,j,k,h;
	double * LMD=new double[m]; 
	double temp=1,sum=0;

	//LMD[i] = (1-lmd)*pow(lmd, m - i) /(1 - pow(lmd, m))
	for(i=m-1;i>=0;i--)
	{
		LMD[i]= temp;
		sum+=temp; 
		temp*=lmd; 
	}
	for(i=0;i<m;i++) LMD[i]/=sum;

	for(i=0;i<n;i++)
	{
		sum=0;
		for(j=0;j<m;j++)
		{
			sum +=x[n*j+i];// LMD[j]* 
		}
		avg[i]=sum;
	}

	for(i=0;i<n;i++)
	{
		for(j=0;j<=i;j++)
		{
			h=i*n+j;
			cov[h]=0;
			for(k=0;k<m;k++)
			{
				//cov[h]+=LMD[k]*x[k*n+i]*x[k*n+j];
				cov[h]+=LMD[k]* (x[k*n+i] - avg[i]) * (x[k*n+j] - avg[j]);

			}
			//cov[h]-=avg[i]*avg[j];
			cov[j*n+i]=cov[h]; 
		}
	}

	delete[] LMD;
	return 0;
}

double var_of_comb(double * coeff,double * cov, int n)
{

	double * buffer= new double [n];
	int i;
	double result;
	for(i=0;i<n;i++)
	{
		buffer[i]=sclprd(coeff, cov+n*i, n);

	}
	result=sclprd(buffer, coeff, n);
	delete [] buffer;
	return result;
}

//��̬�ֲ���ͬ���Ŷȶ�Ӧ��Zֵ
double percentile(double alpha)
{
	switch ((int)(alpha*100))
	{
	case 10: 
		return -1.28;
	case 5:
		return -1.64;
	case 1:
		return -2.33;
	default: return 0;
	}
}

int normal_method(double * sample,
				  int n, int m,
				  double lmd,
				  double * coeff,
				  int T, 
				  double alpha,
				  double * var, 
				  double * cvar,
				  double * average
				  )
{
	double * cov= new double [n*n]; 
	double variant;
	double z_alpha;
	double * avg= new double[n]; 


	covar(sample, n, m, lmd, cov, avg);

	variant=var_of_comb(coeff,cov, n);

	variant*=T;
	if (!(z_alpha=percentile(alpha))) return -3;
	*var=z_alpha*sqrt(variant);
	*cvar=-0.39894*exp(-z_alpha*z_alpha*0.5)*sqrt(variant)/alpha;
	*average=sclprd(avg, coeff, n);
	delete [] avg;
	delete [] cov;
	return 0;
}

//�����Ŷ�ȡ��Varֵ
void percentile_from_dencity(double w[][2], 
							 int n, double alpha, 
							 double * VAR, double * CVAR)
{	

	int i,j;
	double t0,t1,sum=0;

	for(i=0;i<n;i++)
	{
		sum+=w[i][1];
	}
	for(i=0;i<n;i++)
	{
		w[i][1]/=sum;
	}


	for(i=0;i<n-1;i++)
	{
		for(j=i+1;j<n;j++)
		{
			if(w[i][0] > w[j][0])
			{
				t0=w[i][0]; t1=w[i][1];
				w[i][0]=w[j][0]; w[i][1]=w[j][1];
				w[j][0]=t0; w[j][1]=t1;
			}
		}
	}
/*
	CString strText;
	CString strTemp;
	strText = _T("M2#Sorted\r\n");
	for (i = 0; i < n; i++)
	{
		strTemp.Format(_T("%f\t%f\t"), w[i][0], w[i][1]);
		strText += strTemp;
	}
	CTestLog::WiriteToLogFile(strText);
*/
	t0=0;
	for(i=0;i<n;i++)
	{
		t0 += w[i][1];
		if (t0 > alpha ) break;		
	}
	*VAR=w[i][0];

	t0=0;
	t1=0;
	for(j=0; j <= i; j++)
	{
		t1 += w[j][0]* w[j][1]; 
		t0 += w[j][1];
	}
	*CVAR= t1/t0;
}

// ��ʷģ�ⷨ���� var �� cvar
void history_method(
					double *sample,
					int n,
					int m,
					double lmd,
					double *coeff,
					int T, 
					double alpha,
					double *var, 
					double *cvar
				  )
{
	int i,j;
	double (*w) [2]=new double[m][2];
	double temp;
	m=m+1-T;

	for(i=0;i<m;i++)
	{
		for(j=0;j<n;j++)
		{
			sample[i*n+j]=coeff[j]*(exp(sample[i*n+j])-1);

		}
	}

	for(i=0;i<m;i++)
	{
		w[i][0]=0;
		for(j=0;j<n;j++)
		{
			w[i][0]+= sample[i*n+j];
		}		
	}
	temp=1;
	for(i=m-1;i>=0;i--)
	{
		w[i][1]=temp;
		temp*=lmd;
	}
/*
	strText = _T("M2#\r\n");
	for (i = 0; i < m; i++)
	{
		strTemp.Format(_T("%f\t"), w[i][0], w[i][1]);
		strText += strTemp;
	}
	CTestLog::WiriteToLogFile(strText);
*/
	percentile_from_dencity( w , m, alpha,var, cvar);
	delete [] w;
}

void monte_carlo_method(
					double *sample,//
					int n, int m,
					int M,
					double lmd,
					double * coeff,
					int T, 
					double alpha,
					double * var, 
					double * cvar
					)
{
	double * cov= new double [n*n]; 
	double * avg= new double[n]; 
	double * vect_arry=new double[M*n];
	double (*w)[2] =new double[M][2];

	int i = 0;
	int j = 0;

	//step 2 3
	covar(sample, n, m, lmd, cov, avg); //����Э�������

	//step 4 5
	gen_norm_vect(cov,n,M,vect_arry);

	//step 6 
	for(i=0;i<M;i++)
	{
		for(j=0;j<n;j++)
		{
			vect_arry[i*n+j]*=sqrt((double)T);
			vect_arry[i*n+j]+=avg[j]*T;
		}
	}
	for(i=0;i<M;i++)
	{
		w[i][0]=0;
		w[i][1]=1;
		for(j=0;j<n;j++)
		{
			w[i][0]+=vect_arry[i*n+j]*coeff[j];
		}
	}
	percentile_from_dencity(w, M, alpha, var, cvar);

	delete [] w;
	delete [] vect_arry;
	delete [] avg;
	delete [] cov;
}

//������������ת������������
void precac(int type, int m, int n, double * y,	int span)
{
	int i = 0;
	int j = 0;

	switch(type)
	{
	case 0: //0-����
		for(i = 0; i < n; i++)
		{
			for(j = 0; j < m-span; j++)
			{
				y[j*n+i] = y[(j+span)*n+i]-y[j*n+i];
			}
		}
		break;
	case 1://1-�ٷֱ�������
		for(i=0;i<n;i++)
		{
			for(j=0;j<m-span;j++)
			{
				y[j*n+i]=(y[(j+span)*n+i]-y[j*n+i])/y[j*n+i];
			}
		}
		break;
	case 2://2-����������
		for(i=0;i<n;i++)
		{
			for(j=0;j<m-span;j++)
			{
				y[j*n+i]=log(y[(j+span)*n+i])-log(y[j*n+i]);
			}
		}
		break;
	case 3:
		for(i=0;i<n;i++)
		{
			for(j=0;j<m-span;j++)
			{
				y[j*n+i]=y[(j+span)*n+i];
			}
		}
		break;
	}

	return;
}

//���ֲ��������������һ��ļ۸�ת���ɳֲ�ͷ��
void transform_coeff(double *coeff, int m, int n, double *sample)
{
	for(int i = 0;i<n;i++)
	{
		coeff[i] *= sample[n*m-n+i];
	}
}

void caculate_var(
		double *sample,//n��ͷ��������,m����������m��n��, ��0������Զ��һ��
		int n,
		int m,
		int M,// ģ�����,��M�����ģ��
		double lmd,// ˥������,���ȣ�����ȱ�˥��Ȩ��
		double *coeff,//n��ͷ�������������
		int T, //Ԥ��δ����������
		double alpha,//���Ŷ�,<0.5,����0.05
		double * var, //����ֵ
		double * cvar,//����ֵ
		int method,//0������̬����1������ʷģ�ⷨ,2�������ؿ���
		int type//0-���棬1-�ٷֱ������ʣ�2-����������
		)
{
	int m1 = m;
	m = m -1;
	double average = 0.0;

	transform_coeff(coeff, m1, n, sample);
	switch(method)
	{
		case 0:	
			{
				precac(type, m1, n, sample,	1);	
				normal_method(sample, n, m, lmd, coeff,	T, alpha, var, cvar, &average);
			}
			break;
		case 1:
			{
				precac(type, m1, n, sample, T);	// ע���ΪT��������
				history_method(sample, n, m, lmd, coeff, T, alpha, var,	cvar);
			}
			break;
		case 2:
			{
				precac(type, m1, n, sample,	1);	
				monte_carlo_method(sample, n, m, M,	lmd, coeff, T, alpha, var, 	cvar);
			}
			break;
		default: break;
	}
}
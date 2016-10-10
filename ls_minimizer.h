#if !defined(_LS_MINIMIZER_H_)
#define _LS_MINIMIZER_H_

#ifdef max
#undef max
#endif

#include <limits>
#include <vector>
#include <cxcore.h>
//#include "mini_solver.h"
#include "ls_observation.h"
#include "growmat.h"
/*
���
״̬����(state �� measurement vector)��Ҳ������С���˷����ƵĲ�����ά����Ϊn(state_size)
�۲�����(observation �� parameter vector �� b)��һ�ι۲�õ���m������(measurement)ֵ��ά��Ϊ
��ʽ(error �� epsilon)��e = f_i(state) - b �� ����f_i()�ǵ�i�ι۲����������:R(N)->R(M)
���ۺ���(cost �� �ܲв�residual)��F(x)=|e|=0.5*e'*e Ϊ����������Ҳ���Ż���Ŀ�꺯��.
�ſ˱Ⱦ���(J)����СΪm*n
������ֵ(H)����СΪn*n  H=J'*J
�ݶ�����(G)��G(x)=J'*f(x)

�㷨��

���˼·��
�������еĹ۲ⶼ����ͬһ�����̼��㣬�Ҵ󲿷ֲ���������ͬ�������observation_manager������ȫ�������ȫ������
*/


class ls_minimizer
{
public:
	enum {MAX_B_SIZE = 4};

public:
	void set_debug_msg_level(int l);
	ls_minimizer();

	virtual ~ls_minimizer();

	// ����״̬
	double * get_state(){return _state;}
	// ��ʼ��
	int init(int state_dims, int obs_dims, int data_dims, observation_manager * obs_mgr);
	// �����û�����
	void set_user_data(int slot_index, void * ptr);
	// ��LM�㷨��С��
	int minimize_using_levenberg_marquardt_from(double * initial_state);
	// ����в�
	double compute_residual(double * state);

	// LM�㷨��������
	void lm_set_initial_lambda(double l){lm_initial_lambda = l;}
	void lm_set_max_iterations(int it){	lm_max_iterations = it;}
	void lm_set_max_failures_in_a_row(int f){lm_max_failures_in_a_row = f;}
	void lm_set_tol(double t){lm_tol = t;}
protected:
	// ��ʼ����־
	int _init_flag;
	// �û��Զ������ݣ�����eval_func
	void * _user_data[10];
	// �۲���������
	observation_manager * _obs_mgr_ptr;
	// ��������ά��
	int _state_dims;
	// �۲�������ά��
	int _obs_dims; 
	// ����������ά��
	int _data_dims;

	// ״̬
	double * _state, * _lm_state;
	grow_mat _mStates;

	// �ſ˱Ⱦ���(M*N)
	grow_mat _mJ;
	// epsilon������(M*1) (eps_previous)
	grow_mat _mEps;
	// ��С���������Hessian������JtJ���������� (N*N)
	grow_mat _mHessian;
	// �󲽳������Է��̵��Ҷ�(F'(x)=J'f(x)) (N*1)
	grow_mat _mGradient;
	// ���������(N*1)
	grow_mat _mDs;

	// LM�㷨
	// ��Ϻ������󣬴�СΪN*N
	grow_mat _mAug_H;
	// ��ʼlambdaֵ
	double lm_initial_lambda;
	// ��Ϸ��� 0:Ĭ��
	int lm_augment_method;
	// ����������
	int lm_max_iterations;
	// �޸�lambda���ʧ�ܴ���
	int lm_max_failures_in_a_row;
	// �˳���������С����ֵ
	double lm_tol;

protected:
	// ȫ�����Ϊȱʡֵ
	void set_default_values(void);
	// ��������ڴ�
	int alloc_matrices();
	// �ͷ������ڴ�
	void free_matrices(void);
	// ������״̬
	void set_state(double * state, const double *new_state);
	// �����ſ˱Ⱦ���_mJ���ݶ�_mGradient����������_mHessian�Ͳв�residual
	double build_J_G_H_r(double * state);
	// ����LM��Ͼ���
	int compute_augment_H(double lambda);

	// ��ʾ״ֵ̬
	void show_state(int vl, double * state, double r);
};


#endif

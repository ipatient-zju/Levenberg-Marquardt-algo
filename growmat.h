// �Զ������ľ���

#ifndef GROW_MAT_H
#define GROW_MAT_H

#include "cv.h"

class grow_mat : public CvMat 
{
public:
	grow_mat();
	// ���⹹�캯��������data���ݵ���ǰ�����÷�����CvMAT����ͬ���Ǳ�������������data
	grow_mat(int r, int c, int type=CV_64FC1, void* data = 0, int step = CV_AUTOSTEP);
	~grow_mat();

	// ��ʼ��
	int init(int r, int c, int type=CV_64FC1, int maxrows=0, int maxcols=0);
	// �ı��С
	int resize(int r, int c);
	// �����洢�ռ�
	int reserve(int maxrows, int maxcols);
	// ������ʵ�ľ���ָ��
	inline CvMat * get_ptr(){return _mat;}
	// �ͷ��ڴ�
	void free();
	// ���ò���
	inline void set_expand_factor(double f){if(f>1.f) _expand_factor = f;}
	inline void set_copy_flag(int c){_copy_flag=c;}
	inline void set_zero_flag(int z){_zero_flag=z;}
private:
	// ��ʵ�ľ��󣬱���ʵ���ڴ�
	CvMat * _mat;
	// ����ϵ��
	double _expand_factor;
	// ����ʱ�Ƿ�������
	int _copy_flag;
	// ����ʱ���Ƿ����
	int _zero_flag;

	void clear_all(void);
};

#endif

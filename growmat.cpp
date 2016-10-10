
#include "growmat.h"

grow_mat::grow_mat()
{
	clear_all();
}
//
grow_mat::grow_mat(int r, int c, int type, void* data, int step)
{
	clear_all();
	if (data)
	{
		CvMat temp_mat;
		cvInitMatHeader(&temp_mat, r, c, type, data, step );
		_mat = cvCreateMat(r, c, type);
		cvCopy(&temp_mat,_mat);
		cvGetSubRect(_mat, this, cvRect(0,0,c,r));
		return;
	}
	init(r,c,type,0,0);
}

grow_mat::~grow_mat()
{
	if(_mat) 
		cvReleaseMat(&_mat);
}

void grow_mat::clear_all(void)
{
	memset(this,0,sizeof(CvMat));
	_mat = 0;
	_expand_factor = 1.6;
	_copy_flag = 1;
	_zero_flag = 1;
}

int grow_mat::init(int r, int c, int type, int maxrows, int maxcols)
{
	int no_max = 0;
	if (maxrows==0) {maxrows=r*_expand_factor;no_max=1;}
	if (maxcols==0) {maxcols=c*_expand_factor;no_max=1;}
	if (type==0) type=CV_64FC1;
	if (r<=0 || c<=0 || maxrows<0 || maxcols<0 || r>maxrows || c>maxcols)
		return 0;

	// Ϊ�˺�mini_solver��set_solverһ�£������ٴγ�ʼ����
	if (_mat)
	{
		// ������_mat���ڣ���������ͬ����ʾ�ͻ���ı��С��
		int mat_type = CV_MAT_TYPE(_mat->type);
		if (CV_MAT_TYPE(type)==mat_type)
		{
			return resize(r,c);
		}
	}

	if(_mat) cvReleaseMat(&_mat);
	if (no_max)
		_mat = cvCreateMat(r, c, type);
	else
		_mat = cvCreateMat(maxrows, maxcols, type);
	if (_mat==0) return 0;
	if (_zero_flag) cvSetZero(_mat);
	cvGetSubRect(_mat, this, cvRect(0,0,c,r));
	return 1;
}

int grow_mat::resize(int r, int c)
{
	if (_mat==0 || r<0 || c<0)
		return 0;

	if (r <= _mat->rows && c <= _mat->cols) 
	{
		cvGetSubRect(_mat, this, cvRect(0,0,c,r));
	}
	else 
	{
		// ����չ����Ϊԭ����Ҫ�ľ����С��_expand_factor��
		int maxrows = (r > _mat->rows ? r*_expand_factor : _mat->rows);
		int maxcols = (c > _mat->cols ? c*_expand_factor : _mat->cols);
		reserve(maxrows,maxcols);
		cvGetSubRect(_mat, this, cvRect(0,0,c,r));
	}
	return 1;
}

int grow_mat::reserve(int maxrows, int maxcols)
{
	if (_mat==0 || maxrows<0 || maxcols<0)
		return 0;

	// ֻҪ����ľ�����л��д��ڵ�ǰ����ͷ����ڴ�
	if (maxrows > _mat->rows || maxcols > _mat->cols) 
	{
		// �¾���
		CvMat * nm = cvCreateMat(maxrows, maxcols, _mat->type);
		if (nm==0)
			return 0;
		if (_zero_flag) cvSetZero(nm);
		if (this->rows && this->cols && _copy_flag)
		{
			// ����ǰ�����С��Ϊ0������Ҫ�������ݣ��򽫵�ǰ����this�����ݸ��Ƶ�nm
			CvMat sub;
			cvGetSubRect(nm, &sub, cvRect(0,0,this->cols, this->rows));
			cvCopy(this, &sub);
		}
		cvReleaseMat(&_mat);
		_mat = nm;
	}
	return 1;
}

void grow_mat::free()
{
	if(_mat) 
		cvReleaseMat(&_mat);
	_mat=0;
	memset(this,0,sizeof(CvMat));
}
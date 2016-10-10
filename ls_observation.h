

#ifndef LS_OBSERVATION_TYPES_H
#define LS_OBSERVATION_TYPES_H

#include <vector>
#include "cv.h"

class observation;
using namespace std ;
typedef std::vector<observation *> observation_vector;

//! �۲���������
/*! 
�ڴ�ع��ܣ�����observaton����ķ��䣬��������չ���ͷţ�
Ŀǰֻ��������observation��С�̶��������ͬʱ������һ�����������С�

buffer�ķ��䣺( �۲����+�ڴ���Ƿ�ʹ�ñ��+�۲�����+��������) + ָ������
_observation_count*(_one_observation_size+sizeof(bool)+(_obs_dims*sizeof(double)+_data_dims*sizeof(double))+sizeof(void *) )

*/

class observation_manager
{
public:
	observation_manager();
	~observation_manager();

	enum OBS_MGR_METHOD {FixedSize/*,FixedMaxSize,ChangeableSize*/};
	void set_debug_msg_level(int l);

	// (1)��ʼ��
	int init(int state_dims, int obs_dims, size_t obs_size, int data_dims, int reserve_n, OBS_MGR_METHOD method = FixedSize);
	// �۲�������ά��
	inline int get_obs_dims(){return _obs_dims;}
	// ����������ά��
	inline int get_data_dims(){return _data_dims;}
	// ����ָ��vector��ָ��
	observation_vector * get_obs_vector();
	// ���ض������
	inline int get_obs_count(){return _observations.size();}

	// (2)������ӹ۲����
	// ��ʼ
	int begin_add_obs();
	// ���1������
	int add_next_obs(observation * o);
	// ����
	int end_add_obs();
	// ���
	int clear_all_obs();
	// ��ʾ������Ϣ
	void disp_all_obs();

	// (2). �ڴ�غ���
	// Ԥ��n���۲������ڴ�ռ�
	int pool_reserve(int n, bool copy = false);
	// �����ڴ棨����new���ã�
	void * pool_alloc(size_t size);
	// �ͷ��ڴ棨����delete���ã�
	void pool_free(void * p, size_t size);

	// ��Ӷ������
	int add_n_obs_values(double * v, double * d, int size){return 0;}
	// ����ָ���i�������ָ��
	int get_obs_ptr(int i){return 0;}
	// ����
	void sort(void){;}
protected:
	// �����ڴ���з���C++����Ĵ�С���������ݣ�
	void pool_set_element_size(size_t s);
protected:
	// ��ʼ����־
	int _init_flag;
	// (1) ���������й۲����ͬ������
	// �۲�������ά��
	int _obs_dims; 
	// ����������ά��
	int _data_dims;
	// ��������ά��
	int _state_dims;


	// (3) �ڴ������ڲ�����
	// ��ǰ�۲�����ָ������
	observation_vector _observations;

	// (4). �ڴ��
	// ָ��C++�۲������ʼλ�õ�ָ��
	char * _pool_ptr;
	// ָ���Ƿ�ʹ�ñ����ʼλ�õ�ָ��
	char * _pool_used_ptr;
	// ָ��۲�ֵ��ʼλ�õ�ָ��
	char * _pool_obsv_ptr;
	// ָ������ֵ��ʼλ�õ�ָ��
	char * _pool_data_ptr;

	// �۲�����С��C++�۲�����С�����ֽڣ�
	size_t _element_size;
	// ÿԪ��ռ���ڴ��С���ֽڣ�
	size_t _element_total_size;
	// �ڴ�ش�С���ֽڣ�
	size_t _pool_total_size;
	// �ڴ�ؿ������ɵ���������
	int _pool_max_count;
	// ��һ���հ�λ��
	int _pool_next_index;
	// �����ڴ��ģʽ��mode=0�� mode=1�������ģʽ��
	int _pool_alloc_mode;

	// ��������С���ֽڣ�
	size_t _current_buffer_size;
	// ���������������е�(�۲����+�ڴ��Ԫ���Ƿ�ʹ��+����ֵ+����ֵ+ָ������)
	char * _buffer_ptr;
	// ÿ�����·��仺����������(4K)
	int _buffer_increment;
};

//! �۲������
/*! 
һ�ι۲�õ�һ���й�״̬�����Ĺ۲�����. 
ע�⣺�۲��������г�Ա������û�г�ʼ������Ҫ�û�����
ע�⣺�۲�������������ģ���ֻ����ָ�����ݵ�ָ�룬�������ڴ�ķ�����ͷš�
*/
class observation
{
public:
	// ���ù۲�ֵ������ֵ
	inline void set_values(double * v, double * d)
	{
		if(v) memcpy(_obsv, v, _obj_mgr->get_obs_dims()*sizeof(double));
		if(d) memcpy(_data, d, _obj_mgr->get_data_dims()*sizeof(double));
	}
	double * get_obsv(){return _obsv;}
	double * get_data(){return _data;}
	int get_obs_dims(){return _obj_mgr->get_obs_dims();}
	void * operator new(size_t size) throw(bad_alloc);
	void operator delete(void * p, size_t size);// throw();
protected:
	// ָ��۲�����ֵ��ָ��
	double * _obsv;
	// ָ����������ֵ��ָ��
	double * _data;
	// ָ��۲���������ָ��
	static observation_manager * _obj_mgr;

	friend class observation_manager;
	friend class ls_minimizer;
protected:
	// ����ȱʡ����
	void set_default_values(void) { _obsv=0;_data=0;}
	// ���㱾�ι۲�Ĳв�
	double residual(const double *computed_b)
	{
		int n;
		if (_obj_mgr==0) return 0.f;
		n = get_obs_dims();
		double r=0;
		for (int i=0; i<n; i++) {
			//double d = computed_b[i] - _obsv[i];
			double d = _obsv[i] - computed_b[i];
			r += d*d;
		}
		return r;
	}
	// �����������state����۲�
	// state:���뱾�ε������Ƶ�״̬������fx:�������ֵ��J:����ſ˱Ⱦ���user_data:����ָ��ָ���ָ�루ָ�����飩
	virtual int eval_func(const double *state, double *fx, double *J, int J_step, void **user_data) {return 0;}

	// ��ע������3��ָ�룬1��virtualָ�룬��ռ��16�ֽڡ�
private:
	//inline void set_obj_mgr_values(class observation_manager * obj_mgr){if(!_obj_mgr) _obj_mgr=obj_mgr;}
	// virtual int get_nb_measures() {return _obj_mgr->get_obs_dims();}//�����ڱ䳤����
	// ��ǰ�۲�����ά��
	// int _obs_dims;//�����ڱ䳤����
	// observation_manager * _obj_mgr;
	// ���Ŀǰ�۲����Ĳ���ֵ������Ŀǰ�ǹ̶��Ĵ�С��
	//inline int get_nb_measures() {return _obj_mgr->get_obs_dims();}
};

// ģ��̳�
//template <int def_obs_dims, int def_data_dims> class observation2 : public observation<def_obs_dims>


#endif // LS_OBSERVATION_TYPES_H
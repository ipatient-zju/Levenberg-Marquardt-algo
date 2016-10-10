

#include "ls_observation.h"

observation_manager * observation::_obj_mgr = 0;


observation_manager::observation_manager()
{
	_init_flag = 0;
	_state_dims = 0;
	_obs_dims = 0;
	_data_dims = 0;
	_pool_ptr = 0;
	_pool_used_ptr = 0;
	_pool_obsv_ptr = 0;
	_pool_data_ptr = 0;
	
	_element_size = 0;
	_element_total_size = 0;
	_pool_total_size = 0;
	_pool_max_count = 0;
	_pool_next_index = 0;
	_pool_alloc_mode = 0;
	
	_current_buffer_size = 0;
	_buffer_ptr = 0;
	_buffer_increment = 1024*4;

	// Ϊ�۲�����ָ���ڴ�ض����ָ�븳ֵ
	observation::_obj_mgr = this;

}

observation_manager::~observation_manager()
{
	if (_buffer_ptr)
		cvFree(&_buffer_ptr);
}

void observation_manager::set_debug_msg_level(int l)
{

}
int observation_manager::init(int state_dims, int obs_dims, size_t obs_size, int data_dims, int reserve_n, OBS_MGR_METHOD method)
{
	if (state_dims<=0 || obs_dims<0 || obs_size <=0 || data_dims<=0 || method!=FixedSize)
		return 0;
	_obs_dims = obs_dims;
	_data_dims = data_dims;
	_state_dims = state_dims;
	pool_set_element_size(obs_size);
	observation::_obj_mgr = this;
	_init_flag = 1;
	pool_reserve(reserve_n);
	return 1;
}
void observation_manager::pool_set_element_size(size_t s)
{
	_element_size=s;
	_element_total_size = _element_size 
		+ sizeof(char) // ʹ�ñ�־
		+ _obs_dims*sizeof(double) // �۲�ֵ
		+ _data_dims*sizeof(double); // ����ֵ
}

int observation_manager::pool_reserve(int n, bool copy)
{
	if (n<=0 || _init_flag==0)
		return 0;

	char * pool_used_ptr;
	char * pool_ptr;
	char * pool_obsv_ptr;
	char * pool_data_ptr;
	int pool_max_count,pool_total_size,i,j;

	// �����ڴ�ָ��ĺ궨��
	#define SPLIT_MEM_TO_PTR(p) pool_used_ptr = (p);\
	pool_ptr = pool_used_ptr + pool_max_count*sizeof(char);\
	pool_obsv_ptr = pool_ptr + pool_max_count*_element_size;\
	pool_data_ptr = pool_obsv_ptr + pool_max_count*_obs_dims*sizeof(double);


	if (n<=_pool_max_count)
		return 1;
	// �����ڴ���
	i = _element_total_size * n;
	j = (i%_buffer_increment)==0?0:1;
	// 4kΪ�������ڴ���
	pool_total_size = ((int)(i/_buffer_increment)+j)*_buffer_increment;
	// ȡ������������ɵĶ������
	pool_max_count = pool_total_size/_element_total_size;
	
	if (_current_buffer_size < pool_total_size)
	{
		if (copy && _observations.size()>0)
		{
			char * temp_ptr = (char *)cvAlloc(pool_total_size);
			if (temp_ptr ==0 )
				return 0;
			SPLIT_MEM_TO_PTR(temp_ptr);
			for (i=0;i<pool_max_count;i++)
				pool_used_ptr[i]='F';
			// ��ԭ�������ݸ��Ƶ����ڴ�
			for (observation_vector::iterator it = _observations.begin();it != _observations.end(); ++it)
			{
				observation * o = *it;
				bool found = false;
				for (i=0;i<pool_max_count;i++)
				{
					if (_pool_used_ptr[i]=='T' && o ==(observation *) (_pool_ptr + _element_size*i))
					{
						// �����¶����3��ָ��
						char * p_obs = pool_obsv_ptr+_obs_dims*sizeof(double)*i;
						char * p_data = pool_data_ptr+_data_dims*sizeof(double)*i;
						char * p_class = pool_ptr + _element_size*i;
						// �ȸ�����������ݣ��۲���������������
						memcpy(p_class,o,_element_size);
						memcpy(p_obs,o->_obsv,_obs_dims*sizeof(double));
						memcpy(p_data,o->_data,_data_dims*sizeof(double));
						// �޸�ָ��
						o = (observation *)p_class;
						o->_obsv = (double *)p_obs;
						o->_data = (double *)p_data;
						pool_used_ptr[i]='T';
						*it = o;
						found = true;
						break;
					}
				}
				if (found == false)
				{
					cvFree(&temp_ptr);
					return 0;
				}
			}
			if (_buffer_ptr)
				cvFree(&_buffer_ptr);
			_buffer_ptr = temp_ptr;
		}
		else
		{
			if (_buffer_ptr)
				cvFree(&_buffer_ptr);
			_buffer_ptr = (char *)cvAlloc(pool_total_size);
			if (_buffer_ptr ==0 )
				return 0;
			SPLIT_MEM_TO_PTR(_buffer_ptr);
			//for (i=0;i<pool_total_size;i++) _buffer_ptr[i]=0;
			for (i=0;i<pool_max_count;i++)
				pool_used_ptr[i]='F';

		}

		_pool_used_ptr = pool_used_ptr;
		_pool_ptr = pool_ptr; 
		_pool_obsv_ptr = pool_obsv_ptr; 
		_pool_data_ptr = pool_data_ptr;

		_pool_total_size = pool_total_size;	
		_pool_max_count = pool_max_count;
		_current_buffer_size = pool_total_size;
		_observations.reserve(pool_max_count);
	}
	return 1;
}


int observation_manager::begin_add_obs()
{
	if (_init_flag==0)
		return 0;
	int i;

	_observations.clear();
	for (i=0;i<_pool_max_count;i++)
		_pool_used_ptr[i]='F';

	_pool_next_index = 0;
	_pool_alloc_mode = 1;
	return 1;
}

int observation_manager::end_add_obs()
{
	if (_init_flag==0 || _pool_alloc_mode!=1)
		return 0;

	_pool_next_index = 0;
	_pool_alloc_mode = 0;
	return _observations.size();
}

int observation_manager::clear_all_obs()
{
	if (_init_flag==0 || _pool_alloc_mode==1)
	{
		return 0;
	}

	int i;
	_observations.clear();
	for (i=0;i<_pool_max_count;i++)
		_pool_used_ptr[i]='F';

	return 1;
}

int observation_manager::add_next_obs(observation * o)
{
	if (_init_flag==0 || _pool_alloc_mode!=1)
		return 0;
	_observations.push_back(o);
	return 1;
}

void observation_manager::disp_all_obs()
{
	if (_init_flag==0)
		return ;
	printf("----------------------------------------------------");
	printf("\n\nOBS_MGR: disp_all_obs()!\n");
	for (observation_vector::iterator it = _observations.begin();it != _observations.end(); ++it)
	{
		observation * o = *it;
		int i,j;
		printf("obs={");
		for (i=0;i<_obs_dims;i++)
			printf("%g,",o->_obsv[i]);
		printf("\b}, ");
		printf("data={");
		for (i=0;i<_data_dims;i++)
			printf("%g,",o->_data[i]);
		printf("\b}");
		printf("\n");
	}
	printf("----------------------------------------------------");
}

void * observation_manager::pool_alloc(size_t size)
{
	if (_init_flag==0 || size!=_element_size)
	{
		// Ϊ�˱�������û������������ȶ�observation_manager��ʼ��
		throw bad_alloc();
		return 0;
	}

	// ���ڴ�ص�indexλ�÷����ڴ浽ָ��p�ĺ�
	#define ALLOC(p,index) _pool_used_ptr[index] = 'T';\
	p = (observation *) (_pool_ptr + _element_size*index);\
	p->_obsv = (double *) (_pool_obsv_ptr+_obs_dims*sizeof(double)*index);\
	p->_data = (double *) (_pool_data_ptr+_data_dims*sizeof(double)*index);

	observation * p;
	int i;
	if (_pool_alloc_mode==1)
	{
		if (_pool_used_ptr[_pool_next_index] == 'F')
		{
			ALLOC(p,_pool_next_index);
			i = _pool_next_index;
			_pool_next_index ++;
			goto success;
		}
	}
	for (i=0;i<_pool_max_count;i++)
		if (_pool_used_ptr[i] == 'F')
		{
			ALLOC(p,i);
			if (_pool_alloc_mode==1) _pool_next_index = i+1;
			goto success;
		}
	
	// �ڴ治������Ҫ�ٴη����ڴ棡
	if (pool_reserve(_pool_max_count+1,true))
	{
		for (i=0;i<_pool_max_count;i++)
		if (_pool_used_ptr[i] == 'F')
		{
			ALLOC(p,i);
			if (_pool_alloc_mode==1) _pool_next_index = i+1;
			goto success;
		}
	}
	throw bad_alloc();
success:
	return (void *)p;
}

void observation_manager::pool_free(void * p, size_t size)
{
	// Ϊ�˱�����󣬲������û���ʽ����delete�ͷ��ڴ档�û�ʹ��delete��Ч��
	return ;

	// �����������������
	if (p==0 ) return ;
	if(_init_flag==0 || size!=_element_size)
	{
		// :: delete
		return;
	}
	
	int index = (int)((char*)p - _pool_ptr) / _element_size;
	_pool_used_ptr[index] == 'F';
	int i = 0;
	for (observation_vector::iterator it = _observations.begin();it != _observations.end(); ++it,i++)
	{
		void * o = (void *)*it;
		if (o == p)
		{
			_observations.erase(it);
			break;
		}
	}
}


observation_vector * observation_manager::get_obs_vector()
{
	if (_init_flag==0 || _pool_alloc_mode==1)
		return 0;
	return &_observations;
}


void * observation::operator new(size_t size)
{
	return _obj_mgr->pool_alloc(size);
}

void observation::operator delete(void * p, size_t size)
{
	_obj_mgr->pool_free(p,size);
	return ;
}

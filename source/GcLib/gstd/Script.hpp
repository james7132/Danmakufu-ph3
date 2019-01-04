/* ---------------------------------- */
/* 2004..2005 YT�Amkm(�{�̒S���ł���)*/
/* ���̃\�[�X�R�[�h�͎ς�Ȃ�Ă��Ȃ�D���ɂ��냉�C�Z���X�̌��Ŕz�z���܂��B*/
/* A-3�AA-4�ɏ]���A���̃\�[�X��g�ݍ���.exe�ɂ̓��C�Z���X�͓K�p����܂���B*/
/* ---------------------------------- */
/* NYSL Version 0.9982 */
/* A. �{�\�t�g�E�F�A�� Everyone'sWare �ł��B���̃\�t�g����ɂ�����l��l���A*/
/*    �������̍�������̂������̂Ɠ����悤�ɁA���R�ɗ��p���邱�Ƃ��o���܂��B*/
/* A-1. �t���[�E�F�A�ł��B��҂���͎g�p������v�����܂���B*/
/* A-2. �L��������}�̂̔@�����킸�A���R�ɓ]�ځE�Ĕz�z�ł��܂��B*/
/* A-3. �����Ȃ��ނ� ���ρE���v���O�����ł̗��p ���s���Ă��\���܂���B*/
/* A-4. �ύX�������̂╔���I�Ɏg�p�������̂́A���Ȃ��̂��̂ɂȂ�܂��B*/
/*      ���J����ꍇ�́A���Ȃ��̖��O�̉��ōs���ĉ������B*/
/* B. ���̃\�t�g�𗘗p���邱�Ƃɂ���Đ��������Q���ɂ��āA��҂� */
/*    �ӔC�𕉂�Ȃ����̂Ƃ��܂��B�e���̐ӔC�ɂ����Ă����p�������B*/
/* C. ����Ґl�i���� �������� �ɋA�����܂��B���쌠�͕������܂��B*/
/* D. �ȏ�̂R���́A�\�[�X�E���s�o�C�i���̑o���ɓK�p����܂��B */
/* ---------------------------------- */

#if !defined(__SCRIPT_H__)
#define __SCRIPT_H__

#pragma warning (disable:4786)	//STL Warning�}�~
#pragma warning (disable:4018)	//signed �� unsigned �̐��l���r
#pragma warning (disable:4244)	//double' ���� 'float' �ɕϊ�
#include<list>
#include<vector>
#include<string>
#include<map>


//�d���錾�`�F�b�N�����Ȃ�
//#define __SCRIPT_H__NO_CHECK_DUPLICATED

//-------- �ėp
namespace gstd
{
	std::string to_mbcs(std::wstring const & s);
	std::wstring to_wide(std::string const & s);

	template < typename T >
	class lightweight_vector
	{
	public:
		unsigned length;
		unsigned capacity;
		T * at;

		lightweight_vector() : length(0), capacity(0), at(NULL)
		{
		}

		lightweight_vector(lightweight_vector const & source);

		~lightweight_vector()
		{
			if(at != NULL)
			{
				delete[] at;
			}
		}

		lightweight_vector & operator = (lightweight_vector const & source);

		void expand();

		void push_back(T const & value)
		{
			if(length == capacity) expand();
			at[length] = value;
			++length;
		}

		void pop_back()
		{
			--length;

			T temp;
			at[length] = temp;
		}

		void clear()
		{
			length = 0;
		}

		void release()
		{
			length = 0;
			if(at != NULL)
			{
				delete[] at;
				at = NULL;
				capacity = 0;
			}
		}

		unsigned size() const
		{
			return length;
		}

		T & operator[] (unsigned i)
		{
			return at[i];
		}

		T const & operator[] (unsigned i) const
		{
			return at[i];
		}

		T & back()
		{
			return at[length - 1];
		}

		T * begin()
		{
			return & at[0];
		}

		void erase(T * pos);
		void insert(T * pos, T const & value);
	};

	template < typename T >
	lightweight_vector < T >::lightweight_vector(lightweight_vector const & source)
	{
		length = source.length;
		capacity = source.capacity;
		if(source.capacity > 0)
		{
			at = new T[source.capacity];
			for(int i = length - 1; i >= 0; --i)
				at[i] = source.at[i];
		}
		else
		{
			at = NULL;
		}
	}

	template < typename T >
	lightweight_vector < T > & lightweight_vector < T >::operator = (lightweight_vector < T > const & source)
	{
		if(at != NULL) delete[] at;
		length = source.length;
		capacity = source.capacity;
		if(source.capacity > 0)
		{
			at = new T[source.capacity];
			for(int i = length - 1; i >= 0; --i)
				at[i] = source.at[i];
		}
		else
		{
			at = NULL;
		}
		return * this;
	}

	template < typename T >
	void lightweight_vector < T >::expand()
	{
		if(capacity == 0)
		{
			//delete[] at;
			capacity = 4;
			at = new T[4];
		}
		else
		{
			capacity *= 2;
			T * n = new T[capacity];
			for(int i = length - 1; i >= 0; --i)
				n[i] = at[i];
			delete[] at;
			at = n;
		}
	}

	template < typename T >
	void lightweight_vector < T >::erase(T * pos)
	{
		--length;
		for(T * i = pos; i < at + length; ++i)
		{
			* i = * (i + 1);
		}
	}

	template < typename T >
	void lightweight_vector < T >::insert(T * pos, T const & value)
	{
		if(length == capacity)
		{
			unsigned pos_index = pos - at;
			expand();
			pos = at + pos_index;
		}
		for(T * i = at + length; i > pos; --i)
		{
			* i = * (i - 1);
		}
		* pos = value;
		++length;
	}

	//-------- ��������

	class type_data
	{
	public:
		enum type_kind
		{
			tk_real, tk_char, tk_boolean, tk_array
		};

		type_data(type_kind k, type_data * t = NULL) : kind(k), element(t)
		{
		}

		type_data(type_data const & source) : kind(source.kind), element(source.element)
		{
		}

		//�f�X�g���N�^�̓f�t�H���g�ɔC����

		type_kind get_kind()
		{
			return kind;
		}

		type_data * get_element()
		{
			return element;
		}

	private:
		type_kind kind;
		type_data * element;

		type_data & operator = (type_data const & source);
	};

	class value
	{
	public:
		value() : data(NULL)
		{
		}

		value(type_data * t, long double v)
		{
			data = new body();
			data->ref_count = 1;
			data->type = t;
			data->real_value = v;
		}

		value(type_data * t, wchar_t v)
		{
			data = new body();
			data->ref_count = 1;
			data->type = t;
			data->char_value = v;
		}

		value(type_data * t, bool v)
		{
			data = new body();
			data->ref_count = 1;
			data->type = t;
			data->boolean_value = v;
		}

		value(type_data * t, std::wstring v);

		value(value const & source)
		{
			data = source.data;
			if(data != NULL)
				++(data->ref_count);
		}

		~value()
		{
			release();
		}

		value & operator = (value const & source)
		{
			if(source.data != NULL)
			{
				++(source.data->ref_count);
			}
			release();
			data = source.data;
			return * this;
		}

		bool has_data() const
		{
			return data != NULL;
		}

		void set(type_data * t, long double v)
		{
			unique();
			data->type = t;
			data->real_value = v;
		}

		void set(type_data * t, bool v)
		{
			unique();
			data->type = t;
			data->boolean_value = v;
		}

		void append(type_data * t, value const & x);
		void concatenate(value const & x);

		long double as_real() const;
		wchar_t as_char() const;
		bool as_boolean() const;
		std::wstring as_string() const;
		unsigned length_as_array() const;
		value const & index_as_array(unsigned i) const;
		value & index_as_array(unsigned i);
		type_data * get_type() const;

		void unique() const
		{
			if(data == NULL)
			{
				data = new body();
				data->ref_count = 1;
				data->type = NULL;
			}
			else if(data->ref_count > 1)
			{
				--(data->ref_count);
				data = new body(* data);
				data->ref_count = 1;
			}
		}

		void overwrite(value const & source);	//�댯�I�O����ĂԂ�

	private:
		inline void release() 
		{
			if(data != NULL)
			{
				--(data->ref_count);
				if(data->ref_count == 0)
				{
					delete data;
				}
			}
		}
		struct body
		{
			int ref_count;
			type_data * type;
			lightweight_vector < value > array_value;

			union
			{
				long double real_value;
				wchar_t char_value;
				bool boolean_value;
			};
		};

		mutable body * data;
	};

	class script_engine;
	class script_machine;

	typedef value(* callback) (script_machine * machine, int argc, value const * argv);

	struct function
	{
		char const * name;
		callback func;
		unsigned arguments;
	};

	class script_type_manager
	{
	public:
		script_type_manager();

		type_data * get_real_type()
		{
			return real_type;
		}

		type_data * get_char_type()
		{
			return char_type;
		}

		type_data * get_boolean_type()
		{
			return boolean_type;
		}

		type_data * get_string_type()
		{
			return string_type;
		}

		type_data * get_array_type(type_data * element);
	private:
		script_type_manager(script_type_manager const &);
		script_type_manager & operator = (script_type_manager const & source);

		std::list < type_data > types;	//���g�̃|�C���^���g���̂ŃA�h���X���ς��Ȃ��悤��list
		type_data * real_type;
		type_data * char_type;
		type_data * boolean_type;
		type_data * string_type;
	};

	class script_engine
	{
	public:
		script_engine(script_type_manager * a_type_manager, std::string const & source, int funcc, function const * funcv);
		script_engine(script_type_manager * a_type_manager, std::vector<char> const & source, int funcc, function const * funcv);
		virtual ~script_engine();

		void * data;	//�N���C�A���g�p���

		bool get_error()
		{
			return error;
		}

		std::wstring & get_error_message()
		{
			return error_message;
		}

		int get_error_line()
		{
			return error_line;
		}

		script_type_manager * get_type_manager()
		{
			return type_manager;
		}

		//compatibility
		type_data * get_real_type()
		{
			return type_manager->get_real_type();
		}

		type_data * get_char_type()
		{
			return type_manager->get_char_type();
		}

		type_data * get_boolean_type()
		{
			return type_manager->get_boolean_type();
		}

		type_data * get_array_type(type_data * element)
		{
			return type_manager->get_array_type(element);
		}

		type_data * get_string_type()
		{
			return type_manager->get_string_type();
		}

	#ifndef _MSC_VER
	private:
	#endif

		//�R�s�[�A������Z�q�̎��������𖳌���
		script_engine(script_engine const & source);
		script_engine & operator = (script_engine const & source);

		//�G���[
		bool error;
		std::wstring error_message;
		int error_line;

		//�^
		script_type_manager * type_manager;

		//���ԃR�[�h
		enum command_kind
		{
			pc_assign, pc_assign_writable, pc_break_loop, pc_break_routine, pc_call, pc_call_and_push_result, pc_case_begin,
			   pc_case_end, pc_case_if, pc_case_if_not, pc_case_next, pc_compare_e, pc_compare_g, pc_compare_ge, pc_compare_l,
			   pc_compare_le, pc_compare_ne, pc_dup, pc_dup2, pc_loop_ascent, pc_loop_back, pc_loop_count, pc_loop_descent, pc_loop_if,
			   pc_pop, pc_push_value, pc_push_variable, pc_push_variable_writable, pc_swap, pc_yield
		};

		struct block;

		struct code
		{
			command_kind command;
			int line;	//�\�[�X�R�[�h��̍s
			value data;	//pc_push_value��push����f�[�^

			union
			{
				struct
				{
					int level;	//assign/push_variable�̕ϐ��̊��ʒu
					unsigned variable;	//assign/push_variable�̕ϐ��̃C���f�b�N�X
				};
				struct
				{
					block * sub;	//call/call_and_push_result�̔�ѐ�
					unsigned arguments;	//call/call_and_push_result�̈����̐�
				};
				struct
				{
					int ip;	//loop_back�̖߂��
				};
			};

			code()
			{
			}

			code(int the_line, command_kind the_command) : line(the_line), command(the_command)
			{
			}

			code(int the_line, command_kind the_command, int the_level, unsigned the_variable) : line(the_line), command(the_command), level(the_level), variable(the_variable)
			{
			}

			code(int the_line, command_kind the_command, block * the_sub, int the_arguments) : line(the_line), command(the_command), sub(the_sub),
			   arguments(the_arguments)
			   {
			}

			code(int the_line, command_kind the_command, int the_ip) : line(the_line), command(the_command), ip(the_ip)
			{
			}

			code(int the_line, command_kind the_command, value const & the_data) : line(the_line), command(the_command), data(the_data)
			{
			}
		};

		enum block_kind
		{
			bk_normal, bk_loop, bk_sub, bk_function, bk_microthread
		};

		friend struct block;

		typedef lightweight_vector < code > codes_t;

		struct block
		{
			int level;
			int arguments;
			std::string name;
			callback func;
			codes_t codes;
			block_kind kind;

			block(int the_level, block_kind the_kind) : level(the_level), arguments(0), name(), func(NULL), codes(), kind(the_kind)
			{
			}
		};

		std::list < block > blocks;	//���g�̃|�C���^���g���̂ŃA�h���X���ς��Ȃ��悤��list
		block * main_block;
		std::map < std::string, block * > events;

		block * new_block(int level, block_kind kind)
		{
			block x(level, kind);
			return & * blocks.insert(blocks.end(), x);
		}

		friend class parser;
		friend class script_machine;
	};

	class script_machine
	{
	public:
		script_machine(script_engine * the_engine);
		virtual ~script_machine();

		void * data;	//�N���C�A���g�p���

		void run();
		void call(std::string event_name);
		void resume();

		void stop()
		{
			finished = true;
			stopped = true;
		}

		bool get_stopped()
		{
			return stopped;
		}

		bool get_resuming()
		{
			return resuming;
		}

		bool get_error()
		{
			return error;
		}

		std::wstring & get_error_message()
		{
			return error_message;
		}

		int get_error_line()
		{
			return error_line;
		}

		void raise_error(std::wstring const & message)
		{
			error = true;
			error_message = message;
			finished = true;
		}
		void terminate(std::wstring const & message)
		{
			bTerminate = true;
			error = true;
			error_message = message;
			finished = true;
		}

		script_engine * get_engine()
		{
			return engine;
		}

		bool has_event(std::string event_name);

		int get_current_line();

	private:
		script_machine();
		script_machine(script_machine const & source);
		script_machine & operator = (script_machine const & source);

		script_engine * engine;

		bool error;
		std::wstring error_message;
		int error_line;

		bool bTerminate;

		typedef lightweight_vector < value > variables_t;
		typedef lightweight_vector < value > stack_t;

		struct environment
		{
			environment * pred, * succ;	//�o���������N���X�g
			environment * parent;
			int ref_count;
			script_engine::block * sub;
			unsigned ip;
			variables_t variables;
			stack_t stack;
			bool has_result;
		};

		std::list<environment *> call_start_parent_environment_list;
		environment * first_using_environment;
		environment * last_using_environment;
		environment * first_garbage_environment;
		environment * last_garbage_environment;
		environment * new_environment(environment * parent, script_engine::block * b);
		void dispose_environment(environment * object);

		typedef lightweight_vector < environment * > threads_t;

		threads_t threads;
		unsigned current_thread_index;
		bool finished;
		bool stopped;
		bool resuming;

		void yield()
		{
			if(current_thread_index > 0)
				--current_thread_index;
			else
				current_thread_index = threads.size() - 1;
		}

		void advance();

	public:
		int get_thread_count(){return threads.size();}
	};

	template < int num >
	class constant
	{
	public:
		static value func(script_machine * machine, int argc, value const * argv)
		{
			return value(machine->get_engine()->get_real_type(), (long double) num);
		}
	};

}

#endif

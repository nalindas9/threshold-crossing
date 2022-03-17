#pragma once

#include <thread>
#include <functional>
#include <deque>
#include <memory>
#include <mutex>
#include <chrono>
#include <vector>

namespace simdata
{
	template <typename T>
	using ptr = std::shared_ptr<T>;

	template <typename T>
	class worker_queue
	{
	public:
		//struct get_simulated_data() {
		//	return simulated_data;
		//}

		ptr<T> pop_oldest(bool& abort)
		{
			std::unique_lock<std::mutex> lock{ thread_guard };

			while (!abort && inner_queue.empty())
				condition_var.wait_for(lock, std::chrono::milliseconds(10));

			if (abort)
			{
				return nullptr;
			}
			else
			{
				ptr<T> result = inner_queue.front();
				inner_queue.pop_front();
				return result;
			}
		}

		ptr<T> try_pop_oldest()
		{
			std::unique_lock<std::mutex> lock{ thread_guard };

			if (inner_queue.empty())
			{
				return nullptr;
			}
			else
			{
				ptr<T> result = inner_queue.front();
				inner_queue.pop_front();
				return result;
			}
		}

		ptr<T> pop_newest(bool& abort)
		{
			std::unique_lock<std::mutex> lock{ thread_guard };

			while (!abort && inner_queue.empty())
				condition_var.wait_for(lock, std::chrono::milliseconds(10));

			if (abort)
			{
				return nullptr;
			}
			else
			{
				ptr<T> result = inner_queue.back();
				inner_queue.pop_back();
				return result;
			}
		}

		ptr<T> try_pop_newest()
		{
			std::unique_lock<std::mutex> lock{ thread_guard };

			if (inner_queue.empty())
			{
				return nullptr;
			}
			else
			{
				ptr<T> result = inner_queue.back();
				inner_queue.pop_back();
				return result;
			}
		}

		void push(ptr<T> in)
		{
			{
				std::unique_lock<std::mutex> lock{ thread_guard };

				while (inner_queue.size() >= max_queue_size)
				{
					inner_queue.pop_front();
					data_loss_count++;
				}

				inner_queue.push_back(in);
			}

			condition_var.notify_all();
		}

		void clear()
		{
			std::unique_lock<std::mutex> lock{ thread_guard };

			while (!inner_queue.empty())
				inner_queue.pop_front();
		}

		size_t size_hint() const
		{
			return inner_queue.size();
		}

		size_t get_data_loss_count() const
		{
			return data_loss_count;
		}

		void reset_data_loss_count()
		{
			std::unique_lock<std::mutex> lock{ thread_guard };
			data_loss_count = 0;
		}

		size_t max_queue_size = 100;
	private:
		std::deque<ptr<T>> inner_queue;
		std::condition_variable condition_var;
		std::mutex thread_guard;
		size_t data_loss_count = 0;
	};

	template <typename input_t>
	class worker_thread
	{
	public:
		using func_t = std::function<void(ptr<input_t>)>;

		template <typename in_func_t>
		worker_thread(in_func_t&& f, worker_queue<input_t>* input_queue)
			:
			f(std::move(f)),
			input_queue(input_queue),
			inner_thread(&worker_thread::thread_fn, this)
		{
			static_assert(
				std::is_convertible_v<in_func_t, func_t>,
				"Parameter must be convertible to std::function<void(ptr<input_t>)>");
		}

		~worker_thread()
		{
			closing = true;

			while (!thread_exited)
				std::this_thread::yield();

			if (inner_thread.joinable()) inner_thread.join();
		}

		bool process_newest_data_first = false;
	private:
		worker_queue<input_t>* input_queue = nullptr;
		func_t f;
		std::thread inner_thread;
		bool closing = false;
		bool thread_exited = false;

		void thread_fn()
		{
			ptr<input_t> input = nullptr;

			while (!closing)
			{
				if (process_newest_data_first)
					input = input_queue->pop_newest(closing);
				else
					input = input_queue->pop_oldest(closing);

				if (input)
					f(input);
			}

			thread_exited = true;
		}
	};

	template <typename input_t>
	class producer_thread
	{
	public:
		using func_t = std::function<ptr<input_t>(void)>;

		template <typename in_func_t>
		producer_thread(in_func_t&& f, worker_queue<input_t>* output_queue)
			: 
			f(std::move(f)), 
			output_queue(output_queue), 
			inner_thread(&producer_thread::thread_fn, this)
		{
			static_assert(
				std::is_convertible_v<in_func_t, func_t>,
				"Parameter must be convertible to std::function<ptr<input_t>(void)>");
		}

		~producer_thread()
		{
			closing = true;

			while (!thread_exited)
				std::this_thread::yield();

			if (inner_thread.joinable()) inner_thread.join();
		}

		double frequency = 1e4;
	private:
		func_t f;
		std::thread inner_thread;
		bool closing = false;
		bool thread_exited = false;
		worker_queue<input_t>* output_queue = nullptr;

		void thread_fn()
		{
			while (!closing)
			{
				auto tick = std::chrono::high_resolution_clock::now();

				output_queue->push(f());
				
				auto tock = std::chrono::high_resolution_clock::now();

				double elapsed_microseconds = std::chrono::duration<double, std::micro>(tock - tick).count();
				const double target_microseconds = 1e6 / frequency;

				while (elapsed_microseconds < target_microseconds)
				{
					tock = std::chrono::high_resolution_clock::now();
					elapsed_microseconds = std::chrono::duration<double, std::micro>(tock - tick).count();
				}
			}

			thread_exited = true;
		}
	};

	struct simulated_data
	{
		std::vector<float> values;
		size_t sequence_id;
	};

	struct simulation_state
	{
		size_t sequence_count = 0;
		size_t data_length = 1500;
		float amp_min = 0.2f;
		float amp_max = 1.0f;
		float amp_period = 3.2f; // seconds
		float peak_start_min = 50;
		float peak_start_max = 1000;
		float peak_period = 2.4f; // seconds
		float peak_length = 20;
		float sim_frequency = 1e4f;

		ptr<simulated_data> produce_data()
		{
			auto result = std::make_shared<simulated_data>();
			result->values.resize(data_length);
			result->sequence_id = sequence_count;

			const float time = (float)sequence_count / sim_frequency;
			const float amp_factor = 0.5f + 0.5f * std::sinf(2.0f * 3.141569f * time / amp_period);
			const float peak_factor = 0.5f + 0.5f * std::sinf(2.0f * 3.141569f * time / peak_period);
			const float amp_current = amp_min + amp_factor * (amp_max - amp_min);
			const size_t peak_start_current = (size_t)(peak_start_min + peak_factor * (peak_start_max - peak_start_min));

			for (size_t i = peak_start_current; i < peak_start_current + peak_length; i++)
				result->values[i] = amp_current;

			sequence_count++;
			return result;
		}
	};
}
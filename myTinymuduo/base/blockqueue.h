#ifndef BLOCK_QUEUE
#define BLOCK_QUEUE

#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;//不是std::unique_ptr
    std::condition_variable data_cond;

    node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    std::unique_ptr<node> pop_head()   
    {
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);//往前移动
        return old_head;
    }
        std::unique_lock<std::mutex> wait_for_data()   
    {
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock,[&] {return head.get() != get_tail(); }); //说明有数据
        return std::move(head_lock);   
    }//因为锁变量是局部的，所以通过移动锁来确保线程之间的锁的一致性
        std::unique_ptr<node> wait_pop_head()
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data()); //获得头锁
                return pop_head();//获得头结点
        }
        std::unique_ptr<node> wait_pop_head(T& value)
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());  
                value = std::move(*head->data);
            return pop_head();
        }

public:

    threadsafe_queue() :  
        head(new node), tail(head.get())
    {}

    threadsafe_queue(const threadsafe_queue& other) = delete;
    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;

    std::shared_ptr<T> wait_and_pop() 
    {
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T& value)  
    {
        std::unique_ptr<node> const old_head = wait_pop_head(value);
    }


    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
    bool try_pop(T& value)
    {
         std::unique_ptr<node> const old_head = try_pop_head(value);
         if(old_head){
            return true;
         }else{
            return false;
         }
    }
    bool empty()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return (head.get() == get_tail());
    }

    void push(T new_value)  
    {
        std::shared_ptr<T> new_data(
            std::make_shared<T>(std::move(new_value)));
            {
        std::unique_ptr<node> p(new node);//开辟了空间，但没储存
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_tail;
            }
            data_cond.notify_all();
    }

       std::unique_ptr<node> try_pop_head()
        {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if (head.get() == get_tail())
            {    
                return std::unique_ptr<node>();
            }
            return pop_head();
        }
        
        std::unique_ptr<node> try_pop_head(T& value)
        {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if (head.get() == get_tail())
            {
                return std::unique_ptr<node>();
            }
            value = std::move(*head->data);
            return pop_head();
        }
};
#endif
//std::shared_ptr<>没必要使用std::move,std::move是转移智能指针的管理权，但我们要储存权
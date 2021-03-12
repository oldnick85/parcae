#ifndef PARCAE_H
#define PARCAE_H

#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cassert>

#include "node.h"

class CParcae
{
public:
    /**
     * @brief Milestone - наступил новый этап
     * @param[in] thread_name - имя потока
     * @param[in] num - номер этапа
     */
    void Milestone(const std::string &thread_name, const uint num)
    {
        m_milestone_mutex.lock();
        PARCAE_LOG("MILESTONE %s:%u # %s\n", thread_name.c_str(), num, m_current_fate->PrintPrevious().c_str());
        std::string new_thread;
        if (auto next_this = m_current_fate->FindNext(thread_name, num))
        {
            PARCAE_LOG("    FOUND\n");
            std::string next_thread;
            for (const auto &th_name : m_thread_names)
            {
                if (not m_threads.IsReady(th_name))
                    continue;
                if (auto next = next_this->FindNext(th_name))
                    if (next->IsDeadEnd())
                        continue;
                next_thread = th_name;
                break;
            }
            if (next_thread.empty())
                printf("\n\n==== NO THREAD ====\n\n");
            m_current_fate = next_this;
            new_thread = next_thread;
        }
        else
        {
            PARCAE_LOG("    NOT FOUND\n");
            for (const auto &th_name : m_thread_names)
            {
                if (not m_threads.IsReady(th_name))
                    continue;
                if (auto next = m_current_fate->FindNext(th_name))
                {
                    if (not next->IsDeadEnd())
                    {
                        m_current_fate = next;
                        break;
                    }
                }
                else
                {
                    MakeCurrent(thread_name, num);
                    break;
                }
            }
            new_thread = m_current_fate->ThreadName();
        }
        m_milestone_mutex.unlock();
        ContinueThread(thread_name, new_thread);
    }
    /**
     * @brief Start - запуск анализируемых потоков
     * @param[in] func - запускаемая функция (эта функция должна запустить анализируемые потоки)
     * @param[in] thread_names - имена потоков
     */
    void Start(std::function<void()> func, const std::vector<std::string> &thread_names)
    {
        PARCAE_LOG("START\n");
        m_root = CParcaeNodePtr(new CParcaeNode());
        m_root->SetReadyThreads(thread_names);
        m_threads.Clear();
        m_thread_names = thread_names;
        while (not m_root->IsDeadEnd())
        {
            NewRound();
            func();
        }
    }
    /**
     * @brief StartThread - вызывается при запуске анализируемого потока
     * @param[in] thread_name - имя потока
     */
    void StartThread(const std::string &thread_name)
    {
        m_milestone_mutex.lock();
        PARCAE_LOG("START THREAD %s\n", thread_name.c_str());
        m_threads.SetReady(thread_name);
        if (m_threads.AllReady())
        {
            PARCAE_LOG("    ALL READY continue %s\n", thread_name.c_str());
            const auto next_th = ChooseNextThread();
            m_milestone_mutex.unlock();
            ContinueThread(thread_name, next_th);
        }
        else
        {
            PARCAE_LOG("    NOT ALL READY pause %s\n", thread_name.c_str());
            m_milestone_mutex.unlock();
            m_threads.Lock(thread_name);
        }
    }
    /**
     * @brief StopThread - остановка потока
     * @param[in] thread_name - имя потока
     * @remark Должна вызываться когда поток завершил выполнение
     */
    void StopThread(const std::string &thread_name)
    {
        m_milestone_mutex.lock();
        m_threads.Unlock(thread_name);
        m_threads.SetNotReady(thread_name);
        m_current_fate->SetReadyThreads(m_threads.GetReady());
        for (const auto &th_name : m_thread_names)
        {
            if (not m_threads.GetThread(th_name).IsRunning())
            {
                m_threads.Unlock(th_name);
                break;
            }
        }
        m_milestone_mutex.unlock();
    }
    /**
     * @brief Stop - вызывается при завершении очередного раунда
     */
    void Stop()
    {
        m_milestone_mutex.lock();
        m_current_fate->SetDeadEnd();
        m_current_fate->CheckDeadEnd();
        m_threads.SetNotReady();
        //PARCAE_LOG("    PATH >>> %s\n", m_current_fate->PrintPrevious().c_str());
        //PARCAE_LOG("    TREE >>> %s\n", m_root->PrintTree().c_str());
        //PARCAE_LOG("    GVIZ >>> \n%s\n", m_root->PrintDOT().c_str());
        m_milestone_mutex.unlock();
    }

private:
    std::string ChooseNextThread()
    {
        for (const auto &th_name : m_thread_names)
        {
            const auto next = m_current_fate->FindNext(th_name);
            if ((not next) or (not next->IsDeadEnd()))
                return th_name;
        }
        return "";
    }

    void NewRound()
    {
        PARCAE_LOG("NEW ROUND\n");
        for (const auto &th_name : m_thread_names)
        {
            m_threads.GetThread(th_name);
            m_threads.Lock(th_name);
            m_threads.SetNotReady(th_name);
        }
        m_current_fate = m_root;
    }

    void ContinueThread(const std::string &th_cur, const std::string &th_run)
    {
        PARCAE_LOG("    %s -> %s\n", th_cur.c_str(), th_run.c_str());
        if (th_cur == th_run)
            return;
        m_threads.Unlock(th_run);
        m_threads.Lock(th_cur);
    }

    void MakeCurrent(const std::string &thread_name, const uint num)
    {
        PARCAE_LOG("    %s += %s:%u\n", m_current_fate->PrintPrevious().c_str(), thread_name.c_str(), num);
        auto old_fate = m_current_fate;
        m_current_fate = CParcaeNodePtr(new CParcaeNode(thread_name, num, m_threads.GetReady()));
        old_fate->AddNext(m_current_fate);
        m_current_fate->HookOn(old_fate);
    }

    CThreads                    m_threads;
    std::vector<std::string>    m_thread_names;
    CParcaeNodePtr              m_root;
    CParcaeNodePtr              m_current_fate;
    mutable std::mutex          m_milestone_mutex;
};
using CParcaePtr = std::shared_ptr<CParcae>;

#endif // PARCAE_H

#ifndef _COMM_ERROR_SUM_H_
#define _COMM_ERROR_SUM_H_


class ErrorSum
{
public:

    ErrorSum(int max_err, int stop_time)
    {
        m_err_cnt = 0;
        m_last_time = 0;
        
        m_max_err = max_err;
        m_stop_time = stop_time;
    }

    bool check()
    {
        if (m_err_cnt >= m_max_err)
        {
            time_t t_now = time(NULL);

            if (t_now >= m_last_time + m_stop_time)
            {
                m_err_cnt = 0;
                m_last_time = t_now;

                return true;
            }

            return false;
        }

        return true;
    }

    void update(int ret)
    {
        if (ret == 0)
        {
            m_err_cnt = 0;
        }
        else
        {
            m_err_cnt++;
        }

        m_last_time = time(NULL);
    }

private:

    int m_err_cnt;      // �����ۼ�
    time_t m_last_time; // �ϴθ���ʱ��
    
    int m_max_err;      // ���������
    int m_stop_time;    // ֹͣ����ʱ��
};


#endif


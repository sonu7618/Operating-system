#include <stdio.h>

#define MAX_PROC 10
#define QUANTUM 4

typedef struct
{
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int first_response_recorded;
    int response_time;
} Process;

int main()
{
    Process p[MAX_PROC] =
    {
        {1, 0, 5, 5, 0, 0, 0, 0, 0},
        {2, 1, 4, 4, 0, 0, 0, 0, 0},
        {3, 2, 2, 2, 0, 0, 0, 0, 0},
        {4, 3, 1, 1, 0, 0, 0, 0, 0}
    };

    int n = 4;

    int gantt_pid[500];
    int gantt_start[500];
    int gantt_end[500];
    int gantt_count = 0;

    int queue[100];
    int front = 0, rear = 0;

    int in_queue[MAX_PROC] = {0};

    int time = 0;
    int completed = 0;

    /* Sort by arrival time */
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (p[j].arrival_time < p[i].arrival_time)
            {
                Process temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }

    queue[rear++] = 0;
    in_queue[0] = 1;
    time = p[0].arrival_time;

    while (completed < n)
    {
        int idx = queue[front++];

        if (p[idx].remaining_time == 0)
            continue;

        if (!p[idx].first_response_recorded)
        {
            p[idx].response_time = time - p[idx].arrival_time;
            p[idx].first_response_recorded = 1;
        }

        int run_time;

        if (p[idx].remaining_time < QUANTUM)
            run_time = p[idx].remaining_time;
        else
            run_time = QUANTUM;

        int start = time;

        time += run_time;
        p[idx].remaining_time -= run_time;

        gantt_pid[gantt_count] = p[idx].pid;
        gantt_start[gantt_count] = start;
        gantt_end[gantt_count] = time;
        gantt_count++;

        for (int j = 0; j < n; j++)
        {
            if (!in_queue[j] &&
                p[j].remaining_time > 0 &&
                p[j].arrival_time <= time &&
                j != idx)
            {
                queue[rear++] = j;
                in_queue[j] = 1;
            }
        }

        if (p[idx].remaining_time > 0)
        {
            queue[rear++] = idx;
        }
        else
        {
            p[idx].completion_time = time;
            p[idx].turnaround_time =
                p[idx].completion_time - p[idx].arrival_time;
            p[idx].waiting_time =
                p[idx].turnaround_time - p[idx].burst_time;

            in_queue[idx] = 0;
            completed++;
        }

        if (front == rear && completed < n)
        {
            int next = -1;

            for (int j = 0; j < n; j++)
            {
                if (p[j].remaining_time > 0)
                {
                    if (next == -1 ||
                        p[j].arrival_time < p[next].arrival_time)
                    {
                        next = j;
                    }
                }
            }

            if (next != -1 && !in_queue[next])
            {
                if (p[next].arrival_time > time)
                    time = p[next].arrival_time;

                queue[rear++] = next;
                in_queue[next] = 1;
            }
        }
    }

    printf("Round Robin Scheduling\n");
    printf("Time Quantum = %d\n\n", QUANTUM);

    printf("PID\tArrival\tBurst\n");

    for (int i = 0; i < n; i++)
    {
        printf("P%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time);
    }

    printf("\nGantt Chart\n");

    for (int i = 0; i < gantt_count; i++)
    {
        printf("| P%d ", gantt_pid[i]);
    }

    printf("|\n");

    printf("%d", gantt_start[0]);

    for (int i = 0; i < gantt_count; i++)
    {
        printf("    %d", gantt_end[i]);
    }

    printf("\n\n");

    printf("PID\tAT\tBT\tCT\tWT\tTAT\tRT\n");

    double total_wt = 0;
    double total_tat = 0;
    double total_rt = 0;

    for (int i = 0; i < n; i++)
    {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].completion_time,
               p[i].waiting_time,
               p[i].turnaround_time,
               p[i].response_time);

        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }

    printf("\nAverage Waiting Time = %.2f\n", total_wt / n);
    printf("Average Turnaround Time = %.2f\n", total_tat / n);
    printf("Average Response Time = %.2f\n", total_rt / n);

    return 0;
}
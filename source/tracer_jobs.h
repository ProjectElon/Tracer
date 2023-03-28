#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "tracer_core.h"
#include "tracer_random.h"

struct world;
struct camera;
struct frame_buffer;

struct trace_rays_job
{
    world          *World;
    camera         *Camera;
    u32             RayBounceCount;
    frame_buffer   *AccumulationFrameBuffer;
    frame_buffer   *FrameBuffer;
    u32             FrameCount;
    random_series  *RandomSeries;
    u32             StartPixelIndex;
    u32             EndPixelIndex;
};

struct work_queue
{
    std::condition_variable WorkSignalCV;
    std::mutex WorkMutex;

    std::atomic< bool > running;
    std::atomic< u32 > JobIndex;
    std::atomic< u32 > TailJobIndex;
    trace_rays_job Jobs[1024];
};

struct thread_storage
{
    random_series Series;
    u8 Padding[128]; // note(harlequin): false sharing will not get the best of me
};

struct job_system
{
    u32 ThreadCount;
    thread_storage ThreadStorage[128];
    work_queue WorkQueue[128];
    std::thread ThreadPool[128];
};

function void
TraceRays(trace_rays_job *Job);

function void
WorkerThread(work_queue *WorkQueue);

function bool
InitializeJobSystem(job_system *JobSystem);

function void
ShutdownJobSystem(job_system *JobSystem);

function void
QueueTraceRaysJobs(job_system *JobSystem,
                   u32 ThreadIndex,
                   trace_rays_job Job);

function bool
AllJobsCompleted(job_system *JobSystem);
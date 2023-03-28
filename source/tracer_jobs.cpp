#include "tracer_jobs.h"
#include "tracer_camera.h"
#include "tracer_framebuffer.h"

function void
TraceRays(trace_rays_job *Job)
{
    for (u32 PixelIndex = Job->StartPixelIndex;
         PixelIndex < Job->EndPixelIndex;
         PixelIndex++)
    {
        const ray& Ray = Job->Camera->Rays[PixelIndex];

        v3 &AccumulatedColor = Job->AccumulationFrameBuffer->Pixels[PixelIndex];
        AccumulatedColor += TraceRay(Ray, Job->World, Job->RayBounceCount, Job->RandomSeries);
        Job->FrameBuffer->Pixels[PixelIndex] = Clamp(AccumulatedColor / (f32)Job->FrameCount, V3(0.0f), V3(1.0f));
    }
}

function void
WorkerThread(work_queue *WorkQueue)
{
    while (true)
    {
        std::unique_lock< std::mutex > Lock(WorkQueue->WorkMutex);
        WorkQueue->WorkSignalCV.wait(Lock, [&]{ return WorkQueue->JobIndex != WorkQueue->TailJobIndex; });

        while (WorkQueue->JobIndex != WorkQueue->TailJobIndex)
        {
            u32 NewJobIndex = (WorkQueue->JobIndex + 1) % ArrayCount(WorkQueue->Jobs);
            trace_rays_job *Job = &WorkQueue->Jobs[WorkQueue->JobIndex];
            TraceRays(Job);
            WorkQueue->JobIndex = NewJobIndex;
        }

        Lock.unlock();
        WorkQueue->WorkSignalCV.notify_one();
    }
}

function bool
InitializeJobSystem(job_system *JobSystem)
{
    u32 ThreadCount = std::thread::hardware_concurrency();
    u32 WorkerThreadCount = ThreadCount - 1;
    Assert(WorkerThreadCount);
    JobSystem->ThreadCount = ThreadCount;

    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
    {
        thread_storage *Storage = JobSystem->ThreadStorage + ThreadIndex;
        Storage->Series = RandomSeries();
    }

    for (u32 ThreadIndex = 0; ThreadIndex < WorkerThreadCount; ThreadIndex++)
    {
        work_queue *WorkQueue = JobSystem->WorkQueue + ThreadIndex;
        WorkQueue->JobIndex = 0;
        WorkQueue->TailJobIndex = 0;

        JobSystem->ThreadPool[ThreadIndex] = std::thread(WorkerThread,
                                                         WorkQueue);
    }

    return true;
}

function void
QueueTraceRaysJobs(job_system *JobSystem,
                   u32 ThreadIndex,
                   trace_rays_job Job)
{
    work_queue *WorkQueue = &JobSystem->WorkQueue[ThreadIndex];

    {
        std::lock_guard< std::mutex > Lock(WorkQueue->WorkMutex);
        u32 NewTailJobIndex = (WorkQueue->TailJobIndex + 1) % ArrayCount(WorkQueue->Jobs);
        WorkQueue->Jobs[WorkQueue->TailJobIndex] = Job;
        WorkQueue->TailJobIndex = NewTailJobIndex;
    }

    WorkQueue->WorkSignalCV.notify_one();
}

function bool
AllJobsCompleted(job_system *JobSystem)
{
    for (u32 ThreadIndex = 0; ThreadIndex < JobSystem->ThreadCount; ThreadIndex++)
    {
        work_queue *WorkQueue = &JobSystem->WorkQueue[ThreadIndex];
        if (WorkQueue->JobIndex != WorkQueue->TailJobIndex)
        {
            return false;
        }
    }
    return true;
}
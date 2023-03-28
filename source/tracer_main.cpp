#include <glad/glad.h>
#include <glad/glad.c>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "tracer_core.h"
#include "tracer_math.h"

function v3
TraceRay(ray Ray,
         const struct world *World,
         i32 BounceCount,
         struct random_series *RandomSeries);

#include "tracer_imgui.cpp"
#include "tracer_math.cpp"
#include "tracer_random.cpp"
#include "tracer_texture.cpp"
#include "tracer_framebuffer.cpp"
#include "tracer_camera.cpp"
#include "tracer_jobs.cpp"

function bool
SavePngImageToDisk(char   *FilePath,
                   color8 *Image,
                   u32     ImageWidth,
                   u32     ImageHeight)
{
    i32 result = stbi_write_png(FilePath,
                                ImageWidth,
                                ImageHeight,
                                3,
                                Image,
                                sizeof(color8) * ImageWidth);
    return result != 0;
}

#define MAX_MATERIAL_COUNT 1024
#define MAX_MESH_COUNT 1024

struct material
{
    v3  Albedo;
    f32 Roughness;
};

struct mesh
{
    sphere Sphere;
    u32    MaterialIndex;
};

struct world
{
    u32      MaterialCount;
    material Materials[MAX_MATERIAL_COUNT];

    u32  MeshCount;
    mesh Meshes[MAX_MESH_COUNT];
};

u32 PushMaterial(world *World, v3 Albedo, f32 Roughness)
{
    Assert(World->MaterialCount < MAX_MATERIAL_COUNT);
    u32 MaterialIndex   = World->MaterialCount++;
    material *Material  = World->Materials + MaterialIndex;
    Material->Albedo    = Albedo;
    Material->Roughness = Roughness;
    return MaterialIndex;
}

mesh* PushSphere(world *World, v3 Center, f32 Radius, u32 MaterialIndex = 0)
{
    Assert(World->MeshCount < MAX_MESH_COUNT);
    u32 MeshIndex       = World->MeshCount++;
    mesh *Mesh          = World->Meshes + MeshIndex;
    Mesh->Sphere        = SphereCenterRadius(Center, Radius);
    Mesh->MaterialIndex = MaterialIndex;
    return Mesh;
}

inline v3 GetSkyColor(const ray &Ray)
{
    f32 T = 0.5f * (Ray.Direction.Y + 1.0f);
    return (1.0f - T) * V3(1.0f) + T * V3(0.5f, 0.7f, 1.0f);
}

function inline bool
RayCastSphere(const ray    &Ray,
              const sphere &Sphere,
              f32          *OutT)
{
    const v3 RayOriginToSphereCenter = Ray.Origin - Sphere.Center;
    // note(harlequin): should we cache Ray.Direction * Ray.Direction ?
    const f32 A = Dot(Ray.Direction, Ray.Direction);
    const f32 HalfB = Dot(RayOriginToSphereCenter, Ray.Direction);
    // note(harlequin): should we cache Sphere.Radius * Sphere.Radius ?
    const f32 C = Dot(RayOriginToSphereCenter, RayOriginToSphereCenter) - Sphere.Radius * Sphere.Radius;
    const f32 DiscriminantOver4 = HalfB * HalfB - A * C;
    const f32 ClosestT = (-HalfB - sqrtf(DiscriminantOver4)) / A;
    *OutT = ClosestT;
    return DiscriminantOver4 > 0.0f && ClosestT > 0.0f;
}

function inline bool
RayCastSphereFast(const ray    &Ray,
                  const sphere &Sphere,
                  f32          *OutT)
{

    const i32 Mask = 0xff;

#if 0
    v4 _RayOrigin = V4(Ray.Origin);
    v4 _RayDirection = V4(Ray.Direction);
    v4 _SphereCenter = V4(Sphere.Center);

    __m128 RayOrigin = _mm_load_ps((const f32*)&_RayOrigin);
    __m128 RayDirection = _mm_load_ps((const f32*)&_RayDirection);
    __m128 SphereCenter = _mm_load_ps((const f32*)&_SphereCenter);
#endif

    // const v3 RayOriginToSphereCenter = Ray.Origin - Sphere.Center;
    __m128 RayOriginToSphereCenter = _mm_sub_ps(Ray._Origin, Sphere._Center);

    // const f32 A = Dot(Ray.Direction, Ray.Direction);
    __m128 RayDirectionSquared = _mm_dp_ps(Ray._Direction, Ray._Direction, Mask);
    const f32 A = ((f32*)&RayDirectionSquared)[0];

    // const f32 HalfB = Dot(RayOriginToSphereCenter, Ray.Direction);
    __m128 HalfBMul = _mm_dp_ps(RayOriginToSphereCenter, Ray._Direction, Mask);
    const f32 HalfB = ((f32*)&HalfBMul)[0];

    // note(harlequin): should we cache Sphere.Radius * Sphere.Radius ?
    // const f32 C = Dot(RayOriginToSphereCenter, RayOriginToSphereCenter) - Sphere.Radius * Sphere.Radius;
    __m128 RayOriginToSphereCenterSquared = _mm_dp_ps(RayOriginToSphereCenter, RayOriginToSphereCenter, Mask);
    const f32 C = ((f32*)&RayOriginToSphereCenterSquared)[0] - Sphere.Radius * Sphere.Radius;

    const f32 DiscriminantOver4 = HalfB * HalfB - A * C;
    const f32 ClosestT = (-HalfB - sqrtf(DiscriminantOver4)) / A;
    *OutT = ClosestT;
    return DiscriminantOver4 > 0.0f && ClosestT > 0.0f;
}

function v3
TraceRay(ray Ray,
         const world *World,
         i32 BounceCount,
         random_series *RandomSeries)
{
    v3 FinalColor = V3(0.0f);
    f32 Scalar = 1.0f;

    while (BounceCount-- > 0)
    {
        i32 ClosestMeshIndex = -1;
        f32 ClosestT         = MAX_F32;

        for (u32 MeshIndex = 0; MeshIndex < World->MeshCount; MeshIndex++)
        {
            const mesh *Mesh = World->Meshes + MeshIndex;

            f32  T   = 0.0f;
            bool Hit = RayCastSphereFast(Ray, Mesh->Sphere, &T);
            if (Hit && T < ClosestT)
            {
                ClosestT         = T;
                ClosestMeshIndex = MeshIndex;
            }
        }

        if (ClosestMeshIndex != -1)
        {
            const mesh *Mesh = World->Meshes + ClosestMeshIndex;
            intersection_info IntersectionInfo = GetRaySphereIntersectionInfo(Ray,
                                                                              Mesh->Sphere,
                                                                              ClosestT);

            const v3       &Point    = IntersectionInfo.Point + IntersectionInfo.Normal * 0.00001f;
            const v3       &Normal   = IntersectionInfo.Normal;
            const material &Material = World->Materials[Mesh->MaterialIndex];

            FinalColor  += Material.Albedo * Scalar;
            v3 NewNormal = Normalize(Normal + Material.Roughness * RandomV3(RandomSeries, -0.5f, 0.5f));
            v3 Reflected = Reflect(Ray.Direction, NewNormal);

            if (Dot(Reflected, Normal) <= 0.0f)
            {
                break;
            }

            Ray = RayOriginDirection(Point, Reflected);
        }
        else
        {
            FinalColor += GetSkyColor(Ray) * Scalar;
            break;
        }

        Scalar *= 0.5f;
    }

    return FinalColor;
}

global_variable u32 GlobalFrameBufferWidth;
global_variable u32 GlobalFrameBufferHeight;

void OnFramebufferResize(GLFWwindow *Window,
                         i32         Width,
                         i32         Height)
{
    GlobalFrameBufferWidth  = Width;
    GlobalFrameBufferHeight = Height;
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "failed to initalize glfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *Window = glfwCreateWindow(1280,
                                          720,
                                          "Tracer",
                                          nullptr,
                                          nullptr);
    Assert(Window);

    i32 FrameBufferWidth;
    i32 FrameBufferHeight;
    glfwGetFramebufferSize(Window, &FrameBufferWidth, &FrameBufferHeight);
    GlobalFrameBufferWidth  = FrameBufferWidth;
    GlobalFrameBufferHeight = FrameBufferHeight;

    glfwSetFramebufferSizeCallback(Window, OnFramebufferResize);
    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1);

    i32 GladStatus = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    Assert(GladStatus);

    bool ImGuiInitialized = InitializeImGui(Window);
    Assert(ImGuiInitialized);

    frame_buffer AccumulationFrameBuffer = {};
    InitializeFrameBuffer(&AccumulationFrameBuffer, 1280, 720);
    ClearFrameBuffer(&AccumulationFrameBuffer);

    frame_buffer ViewportFrameBuffer = {};
    InitializeFrameBuffer(&ViewportFrameBuffer, 1280, 720);

    const f32 FocalLength = 1.0f;
    const v3 Origin       = V3(0.0f, 0.0f, 0.0f);
    camera ViewportCamera = {};
    InitializeCamera(&ViewportCamera,
                     ViewportFrameBuffer.Width,
                     ViewportFrameBuffer.Height,
                     FocalLength,
                     Origin);

    v3 White = V3(1.0f, 1.0f, 1.0f);
    v3 Blue  = V3(0.5f, 0.7f, 1.0f);

    world World = {};
    PushMaterial(&World, V3(1.0f, 0.0f, 0.0f), 0.0f);
    PushMaterial(&World, V3(0.0f, 1.0f, 0.0f), 0.0f);
    PushMaterial(&World, V3(0.0f, 0.0f, 1.0f), 0.2f);

    PushSphere(&World, V3(0.5f, 0.0f, -1.0f), 0.5f, 0);
    PushSphere(&World, V3(-0.5f, 0.0f, -1.0f), 0.5f, 1);
    PushSphere(&World, V3(0.0f, -100.5f, -1.0f), 100.0f, 2);

    u32 RayBounceCount = 2;
    u32 FrameCount = 1;

    opengl_texture ViewportTexture = {};
    InitializeOpenglTexture(&ViewportTexture,
                            ViewportFrameBuffer.Width,
                            ViewportFrameBuffer.Height,
                            GL_RGB,
                            GL_RGB32F,
                            GL_FLOAT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ViewportTexture.Handle);

    job_system *JobSystem = new(malloc(sizeof(job_system))) job_system {};
    InitializeJobSystem(JobSystem);

    while (!glfwWindowShouldClose(Window))
    {
        glfwPollEvents();

        u32 PixelCount = ViewportFrameBuffer.Width * ViewportFrameBuffer.Height;
        u32 PixelCountPerThread = PixelCount / JobSystem->ThreadCount;

        for (u32 ThreadIndex = 0;
             ThreadIndex < JobSystem->ThreadCount - 1;
             ThreadIndex++)
        {
            u32 StartPixelIndex = PixelCountPerThread * ThreadIndex;
            u32 EndPixelIndex   = PixelCountPerThread * (ThreadIndex + 1);

            trace_rays_job Job  = {};
            Job.World = &World;
            Job.Camera = &ViewportCamera;
            Job.RayBounceCount = RayBounceCount;
            Job.AccumulationFrameBuffer = &AccumulationFrameBuffer;
            Job.FrameBuffer = &ViewportFrameBuffer;
            Job.FrameCount = FrameCount;
            Job.RandomSeries = &JobSystem->ThreadStorage[ThreadIndex].Series;
            Job.StartPixelIndex = StartPixelIndex;
            Job.EndPixelIndex = EndPixelIndex;

            QueueTraceRaysJobs(JobSystem, ThreadIndex, Job);
        }

        trace_rays_job Job = {};
        Job.World = &World;
        Job.Camera = &ViewportCamera;
        Job.RayBounceCount = RayBounceCount;
        Job.AccumulationFrameBuffer = &AccumulationFrameBuffer;
        Job.FrameBuffer = &ViewportFrameBuffer;
        Job.FrameCount = FrameCount;
        Job.RandomSeries = &JobSystem->ThreadStorage[JobSystem->ThreadCount - 1].Series;
        Job.StartPixelIndex = (JobSystem->ThreadCount - 1) * PixelCountPerThread;
        Job.EndPixelIndex = PixelCount;
        TraceRays(&Job);

        while (!AllJobsCompleted(JobSystem));

        FrameCount++;

        CopyFrameBufferToTexture(&ViewportFrameBuffer, &ViewportTexture);

        ImVec2 ViewportSize = {};

        ImGuiBeginFrame();
        {
            {ImGui::Begin("Settings");
            ImGui::SliderInt("RayBounceCount", (i32*)&RayBounceCount, 1, 64);
            ImGui::SliderInt("FrameCount", (i32*)&FrameCount, 1, UINT_MAX);

            ImGuiIO &IO = ImGui::GetIO();
            ImGui::Text("Framerate %.2f ms/frame (%.1f FPS)", 1000.0f / IO.Framerate, IO.Framerate);
            ImGui::End();}

            {ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Viewport");
            ViewportSize = ImGui::GetContentRegionAvail();

            ImGui::Image((ImTextureID)ViewportTexture.Handle,
                         ImVec2((f32)ViewportTexture.Width, (f32)ViewportTexture.Height),
                         ImVec2(0, 0),
                         ImVec2(1, 1));

            ImGui::End();
            ImGui::PopStyleVar();}
        }
        ImGuiEndFrame(GlobalFrameBufferWidth,
                      GlobalFrameBufferHeight);

        u32 ViewportWidth   = (u32)ViewportSize.x;
        u32 ViewportHeight  = (u32)ViewportSize.y;
        if (ViewportWidth  != ViewportFrameBuffer.Width ||
            ViewportHeight != ViewportFrameBuffer.Height)
        {
            FrameCount = 1;

            ResizeCamera(&ViewportCamera,
                          ViewportWidth,
                          ViewportHeight);

            ResizeFrameBuffer(&AccumulationFrameBuffer,
                               ViewportWidth,
                               ViewportHeight);

            ResizeFrameBuffer(&ViewportFrameBuffer,
                               ViewportWidth,
                               ViewportHeight);

            ResizeTexture(&ViewportTexture,
                           ViewportWidth,
                           ViewportHeight);
        }

        if (FrameCount == 1)
        {
            ClearFrameBuffer(&AccumulationFrameBuffer);
        }

        glfwSwapBuffers(Window);
    }

    glfwTerminate();
}
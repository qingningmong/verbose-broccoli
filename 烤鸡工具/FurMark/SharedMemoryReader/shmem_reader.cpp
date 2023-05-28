#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <stdint.h>


#define SHMEM_NAME _T("GPUShark_ShMem")
#define SHMEM_HEADER_SIGNATURE "[Geeks3D_GPUShark]"
#define SHMEM_HEADER_VERSION "[20170514]"


#pragma pack(push, 1)

// Structure for version 20170514.
struct shmem_GPU_t
{
  char name[128];
  char codename[128];
  uint32_t vendor_id;
  uint32_t device_id;
  float core_temperature;
  float core_clock;
  float mem_clock;
  float fan_speed_percent;
  float core_usage;
  float mem_usage;
  float power_percent;
  uint8_t padding[28]; // 320 bytes
};

#define GPUSHARK_SHMEM_MAX_GPUS 4

struct GPUSHARK_SHMEM
{
  char signature[32]; 
  char version[16];
  uint32_t num_gpus;
  shmem_GPU_t gpus[GPUSHARK_SHMEM_MAX_GPUS];
};
#pragma pack(pop)




GPUSHARK_SHMEM shmem;


bool shmem_is_running()
{
  bool ret = false;
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHMEM_NAME);
  if (hMapFile)
  {
    ret = true;
    CloseHandle(hMapFile);
  }
  return ret;
}

uint32_t shmem_get_num_gpus()
{
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHMEM_NAME);
  if (!hMapFile)
    return 0;

  GPUSHARK_SHMEM *p = (GPUSHARK_SHMEM*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
  if (!p)
  {
    CloseHandle(hMapFile);
    return false;
  }

  GPUSHARK_SHMEM shmem;
  memcpy(&shmem, p, sizeof(GPUSHARK_SHMEM));
  UnmapViewOfFile(p);
  CloseHandle(hMapFile);

  return shmem.num_gpus;
}


const char* shmem_get_gpu_name(size_t gpu_index)
{
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHMEM_NAME);
  if (!hMapFile)
    return 0;

  GPUSHARK_SHMEM *p = (GPUSHARK_SHMEM*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
  if (!p)
  {
    CloseHandle(hMapFile);
    return false;
  }

  GPUSHARK_SHMEM shmem;
  memcpy(&shmem, p, sizeof(GPUSHARK_SHMEM));
  UnmapViewOfFile(p);
  CloseHandle(hMapFile);

  return shmem.gpus[gpu_index].name;
}

float shmem_get_gpu_temperature(size_t gpu_index)
{
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHMEM_NAME);
  if (!hMapFile)
    return 0;

  GPUSHARK_SHMEM *p = (GPUSHARK_SHMEM*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
  if (!p)
  {
    CloseHandle(hMapFile);
    return false;
  }

  GPUSHARK_SHMEM shmem;
  memcpy(&shmem, p, sizeof(GPUSHARK_SHMEM));
  UnmapViewOfFile(p);
  CloseHandle(hMapFile);

  return shmem.gpus[gpu_index].core_temperature;
}

bool shmem_read_signature()
{
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHMEM_NAME);
  if (!hMapFile)
    return false;

  GPUSHARK_SHMEM *p = (GPUSHARK_SHMEM*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
  if (!p)
  {
    CloseHandle(hMapFile);
    return false;
  }

  GPUSHARK_SHMEM shmem;
  memcpy(&shmem, p, sizeof(GPUSHARK_SHMEM));

  UnmapViewOfFile(p);
  CloseHandle(hMapFile);

  printf("\nConnected to shared memory block: %s", shmem.signature);
  return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
  if (!shmem_read_signature())
  {
    printf("\nGPU Shark is not running.\nBye!\n");
    return 0;
  }

  printf("\nGPU Shark is running. Connected to shared memory ok.");

  uint32_t ngpus = shmem_get_num_gpus();
  printf("\nNum GPUs: %d", ngpus);

  uint32_t counter = 0;
  while (shmem_is_running())
  {
    uint32_t ngpus = shmem_get_num_gpus();

    for (uint32_t i = 0; i < ngpus; i++)
    {
      const char* name = shmem_get_gpu_name(i);
      float t = shmem_get_gpu_temperature(i);
      if (t)
        printf("\n%.6d - [%s] core temp: %.1f degC", counter, name, t);
      else
        printf("\n%.6d - [%s] - core temp: N.A.", counter, name);

    }
    counter++;
    Sleep(1000);
  }

  printf("\nBye!");
  return 0;
}


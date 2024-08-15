#include "cpuid.h"
#include <stdio.h>

void main()
{
    _p_info pinfo;
    int i;
    i = _cpuid(&pinfo);
    printf("Vendor Name: %s\n",pinfo.v_name);
    printf("CPU Model: %s\n", pinfo.model_name);
    printf("Support for MMX: %c\n", i&_CPU_FEATURE_MMX ? 'Y' : 'N');
    printf("Support for SSE: %c\n", i&_CPU_FEATURE_SSE ? 'Y' : 'N');
    printf("Support for SSE2: %c\n", i&_CPU_FEATURE_SSE2 ? 'Y' : 'N');
    printf("Support for 3DNOW: %c\n", i&_CPU_FEATURE_3DNOW ? 'Y' : 'N');
    printf("OS Support for MMX: %c\n", pinfo.os_support&_CPU_FEATURE_MMX ? 'Y' : 'N');
    printf("OS Support for SSE: %c\n", pinfo.os_support&_CPU_FEATURE_SSE ? 'Y' : 'N');
    printf("OS Support for SSE2: %c\n", pinfo.os_support&_CPU_FEATURE_SSE2 ? 'Y' : 'N');
    printf("OS Support for 3DNOW: %c\n", pinfo.os_support&_CPU_FEATURE_3DNOW ? 'Y' : 'N');
}

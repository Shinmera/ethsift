config = dict()

config['fixed_cpu_frequency'] = "@ 2900 MHz"
config['frequency'] = 2.9e9
config['maxflops_sisd'] = 2
config['maxflops_simd'] = 16 
config['maxflops_sisd_fma'] = 4 
config['maxflops_simd_fma'] = 32 
config['roofline_beta'] = 64        # According to WikiChip (Skylake)
config['figure_size'] = (20,9)
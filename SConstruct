import os

env=Environment(ENV=os.environ, tools=['default', 'protoc', 'grpc'])

print(env['ENV']['HOME'])
print(env['ENV']['PATH'])

if 'CC' in os.environ:
    env['CC']=os.environ['CC']
    
if 'CXX' in os.environ:
    env['CXX']=os.environ['CXX']

root_dir = Dir('#').srcnode().abspath

config = {}

config['cryto_lib_dir'] = root_dir + "/third_party/crypto/library"
config['cryto_include'] = config['cryto_lib_dir']  + "/include"
config['cryto_lib'] = config['cryto_lib_dir']  + "/lib"

config['db-parser_lib_dir'] = root_dir + "/third_party/db-parser/library"
config['db-parser_include'] = config['db-parser_lib_dir']  + "/include"
config['db-parser_lib'] = config['db-parser_lib_dir']  + "/lib"


if 'config_file' in ARGUMENTS:
    SConscript(ARGUMENTS['config_file'], exports=['env','config'])



env.Append(CCFLAGS = ['-fPIC','-Wall', '-march=native'])
env.Append(CXXFLAGS = ['-std=c++14'])
env.Append(CPPPATH = ['/usr/local/include', config['cryto_include'], config['db-parser_include'],'/usr/local/include/cryptopp'])
env.Append(LIBPATH = ['/usr/local/lib', config['cryto_lib'], config['db-parser_lib']])
env.Append(RPATH = [config['cryto_lib'], config['db-parser_lib']])
env.Append(LINKFLAGS = ['-Wl,--copy-dt-needed-entries','-pg','-no-pie','-fno-builtin'])
env.Append(LIBS = ['crypto','ssl', 'sse_crypto', 'grpc++_unsecure', 'grpc', 'protobuf', 'dl', 'sse_dbparser', 'rocksdb', 'snappy', 'z', 'bz2',  'lz4','cryptopp','pthread'])
 
#Workaround for OS X
if env['PLATFORM'] == 'darwin':
    rpathprefix = '-rpath'
    env.Append(LINKFLAGS = [[rpathprefix, lib] for lib in env['RPATH']])
    env.Append(CPPPATH=['/usr/local/opt/openssl/include'])
    env.Append(LIBPATH=['/usr/local/opt/openssl/lib'])    
    env.Append(LIBPATH=['/usr/local/lib/libboost_regex.so'])
    env.Append(LIBPATH=['debug_new.cpp'])
    # env.Append(LINKFLAGS = ['-rpath', cryto_lib_dir+'/lib'])

env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1

env.Append(CCFLAGS = ['-g','-O0', '-pg', '-no-pie', '-fno-builtin'])

static_relic = ARGUMENTS.get('static_relic', 0)

env.Append(CPPDEFINES = ['BENCHMARK'])

def run_test(target, source, env):
    app = str(source[0].abspath)
    if os.spawnl(os.P_WAIT, app, app)==0:
        return 0
    else:
        return 1

bld = Builder(action = run_test)
env.Append(BUILDERS = {'Test' :  bld})


crypto_lib_target = env.Command(config['cryto_lib_dir'], "", "cd third_party/crypto && scons lib static_relic={0}".format(static_relic))
db_parser_target = env.Command(config['db-parser_lib_dir'], "", "cd third_party/db-parser && scons lib")
env.Alias('deps', [crypto_lib_target, db_parser_target])

objects = SConscript('src/build.scons', exports='env', variant_dir='build')

env.Depends(objects["zorionsq"],[crypto_lib_target , db_parser_target])
env.Depends(objects["packzorionsq"],[crypto_lib_target , db_parser_target])
env.Depends(objects["orionsq"],[crypto_lib_target , db_parser_target])
env.Depends(objects["mzorion"],[crypto_lib_target , db_parser_target])
env.Depends(objects["packorion"],[crypto_lib_target , db_parser_target])
env.Depends(objects["baseline"],[crypto_lib_target , db_parser_target])
env.Depends(objects["t111"],[crypto_lib_target , db_parser_target])

Clean(objects["packzorionsq"]+objects["zorionsq"]+objects["orionsq"]+objects["mzorion"]+objects["packorion"]+objects["baseline"]+objects["t111"], 'build')

outter_env = env.Clone()
outter_env.Append(CPPPATH = ['build'])



mzorion_debug_prog   = outter_env.Program('mzorion_debug',    ['test_mzorion.cpp']     + objects["mzorion"])
orionsq_debug_prog   = outter_env.Program('orionsq_debug',    ['test_orionsq.cpp']     + objects["orionsq"])
packzorionsq_debug_prog   = outter_env.Program('packzorionsq_debug',    ['test_packzorionsq.cpp']     + objects["packzorionsq"])
zorionsq_debug_prog   = outter_env.Program('zorionsq_debug',    ['test_zorionsq.cpp']     + objects["zorionsq"])
packorion_debug_prog   = outter_env.Program('packorion_debug',    ['test_packorion.cpp']     + objects["packorion"])
baseline_debug_prog   = outter_env.Program('baseline_debug',    ['test_baseline.cpp']     + objects["baseline"])
t111_debug_prog   = outter_env.Program('t111_debug',    ['t111.cpp'] +objects["t111"])


env.Alias('zorionsq', [zorionsq_debug_prog])
env.Alias('packzorionsq', [packzorionsq_debug_prog])
env.Alias('packorion', [packorion_debug_prog])
env.Alias('mzorion', [mzorion_debug_prog])
env.Alias('orionsq', [orionsq_debug_prog])
env.Alias('baseline', [baseline_debug_prog])
env.Alias('t111', [t111_debug_prog])

env.Default(['packzorionsq'])
env.Default(['zorionsq'])
env.Default(['packorion'])
env.Default(['orionsq'])
env.Default(['mzorion'])
env.Default(['baseline'])
env.Default(['t111'])

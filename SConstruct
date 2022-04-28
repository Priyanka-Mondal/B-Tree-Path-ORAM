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

env.Depends(objects["orionsq"],[crypto_lib_target , db_parser_target])
env.Depends(objects["btree"],[crypto_lib_target , db_parser_target])
env.Depends(objects["wfilebtree"],[crypto_lib_target , db_parser_target])

Clean(objects["orionsq"]+objects["btree"]+objects["wfilebtree"], 'build')

outter_env = env.Clone()
outter_env.Append(CPPPATH = ['build'])



orionsq_debug_prog   = outter_env.Program('orionsq_debug',    ['test_orionsq.cpp']     + objects["orionsq"])
btree_debug_prog   = outter_env.Program('btree_debug',    ['test_btree.cpp']     + objects["btree"])
wfilebtree_debug_prog   = outter_env.Program('wfilebtree_debug',    ['test_wfilebtree.cpp']     + objects["wfilebtree"])


env.Alias('orionsq', [orionsq_debug_prog])
env.Alias('btree', [btree_debug_prog])
env.Alias('wfilebtree', [wfilebtree_debug_prog])

env.Default(['orionsq'])
env.Default(['btree'])
env.Default(['wfilebtree'])

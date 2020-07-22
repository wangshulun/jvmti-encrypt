#include <iostream>
#include <string.h>

#include "jvmtiagent.h"
#include "jvmti.h"
using namespace std;

jvmtiEnv* JvmTIAgent::m_jvmti = 0;
char* JvmTIAgent::m_options = 0;

// 项目包名前缀
const char * g_SelfJavaPackageName = "com/allen/decrypt";

JvmTIAgent::~JvmTIAgent() throw(AgentException)
{
	// 必须释放内存，防止内存泄露
	m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(m_options));
}

void JvmTIAgent::Init(JavaVM *vm) const throw(AgentException) {
	jvmtiEnv *jvmti = 0;
	jint ret = (vm)->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_0);
	if (ret != JNI_OK || jvmti == 0) {
		throw AgentException(JVMTI_ERROR_INTERNAL);
	}
	m_jvmti = jvmti;
}

void JvmTIAgent::ParseOptions(const char* str) const throw(AgentException)
{
	if (str == 0)
		return;
	const size_t len = strlen(str);
	if (len == 0)
		return;

	// 必须做好内存复制工作
	jvmtiError error;
	error = m_jvmti->Allocate(len + 1, reinterpret_cast<unsigned char**>(&m_options));
	CheckException(error);
	strcpy_s(m_options, strlen(str) + 1, str);

	// 可以在这里进行参数解析的工作
	// ...
	cout << "[JVMTI Agent] Application Options: " << m_options << endl;
}

void JvmTIAgent::AddCapability() const throw(AgentException)
{
	// 创建一个新的环境
	jvmtiCapabilities caps;
	memset(&caps, 0, sizeof(caps));
	// 可以生成 方法进入事件
	//caps.can_generate_method_entry_events = 1;
	// 可以对每个加载要类生成 ClassFileLoadHook 事件
	caps.can_generate_all_class_hook_events = 1;

	// 设置当前环境
	jvmtiError error = m_jvmti->AddCapabilities(&caps);
	CheckException(error);
}

void JvmTIAgent::RegisterEvent() const throw(AgentException)
{
	// 创建一个新的回调函数
	jvmtiEventCallbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	// 设置方法进入函数指针
	//callbacks.MethodEntry = &JvmTIAgent::HandleMethodEntry;
	// 设置类加载方法函数指针
	callbacks.ClassFileLoadHook = &JvmTIAgent::HandleClassFileLoadHook;

	// 设置回调函数
	jvmtiError error;
	error = m_jvmti->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
	CheckException(error);

	// 开启事件监听
	error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
	error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, 0);
	CheckException(error);
}

void JNICALL JvmTIAgent::HandleMethodEntry(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jmethodID method)
{
	try {
		jvmtiError error;
		jclass clazz;
		char* name;
		char* signature;

		// 获得方法对应的类
		error = m_jvmti->GetMethodDeclaringClass(method, &clazz);
		CheckException(error);
		// 获得类的签名
		error = m_jvmti->GetClassSignature(clazz, &signature, 0);
		CheckException(error);
		// 获得方法名字
		error = m_jvmti->GetMethodName(method, &name, NULL, NULL);
		CheckException(error);

		cout << "[JVMTI Agent] Enter method" << signature << " -> " << name << "(..)" << endl;

		// 必须释放内存，避免内存泄露
		error = m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
		CheckException(error);
		error = m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
		CheckException(error);

	}
	catch (AgentException& e) {
		cerr << "Error when enter HandleMethodEntry: " << e.what() << " [" << e.ErrCode() << "]";
	}
}

void JNICALL JvmTIAgent::HandleClassFileLoadHook(
	jvmtiEnv *jvmti_env,
	JNIEnv* jni_env,
	jclass class_being_redefined,
	jobject loader,
	const char* name,
	jobject protection_domain,
	jint class_data_len,
	const unsigned char* class_data,
	jint* new_class_data_len,
	unsigned char** new_class_data)
{
	try {
		if (name == NULL) {
			printf("className is null");
			return;
		}
		// class 文件长度赋值
		*new_class_data_len = class_data_len;
		//printf("className %s", name);
		jvmtiError error;
		// 申请 新的class 字符空间
		error = m_jvmti->Allocate(class_data_len, new_class_data);
		//printf("className Allocate完成 %s", name);
		CheckException(error);
		//printf("className %s CheckException SUCCESS", name);
		unsigned char *pNewClass = *new_class_data;
		unsigned int selfPackageLen = strlen(g_SelfJavaPackageName);
		if (strlen(name) > selfPackageLen && 0 == strncmp(name, g_SelfJavaPackageName, selfPackageLen)) {
			// 进行 class 解密 ：规则（奇数异或 0x07, 偶数 异或 0x08, 并换位， 最后一位奇数异或0x09）
			//printf("className %s 开始解密 BEGIN", name);
			int index = 0;
			for (; index < class_data_len - 1; ) {
				*pNewClass++ = class_data[index + 1] ^ 0x08;
				*pNewClass++ = class_data[index] ^ 0x07;
				index += 2;
			}

			if ((0 == index && 1 == class_data_len) || class_data_len - 1 == index) {	// size 1 || size 2n + 1
				*pNewClass = class_data[index] ^ 0x09;
			}
			cout << "[JVMTI Agent] Decrypt class (" << name << ") finished !" << endl;
		}
		else {
			//printf("className %s 开始复制 memcpy BEGIN", name);
			memcpy(pNewClass, class_data, class_data_len);
			//printf("className %s 复制完成 memcpy BEGIN", name);
		}
		//printf("className %s 解密 SUCCESS", name);

	}
	catch (AgentException& e) {
		cerr << "Error when enter HandleClassFileLoadHook: " << e.what() << " [" << e.ErrCode() << "]";
	}
}

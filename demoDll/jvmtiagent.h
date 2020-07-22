#include "jvmti.h"

class AgentException
{
public:
	AgentException(jvmtiError err) {
		m_error = err;
	}

	const char* what() const throw() {
		switch (m_error) {
		case 99:
			return "The capability being used is false in this environment.";
		default:
			return "错误码： https://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html#ErrorSection";
		}

	}

	jvmtiError ErrCode() const throw() {
		return m_error;
	}

private:
	jvmtiError m_error;
};


class JvmTIAgent
{
public:

	JvmTIAgent() throw(AgentException) {}

	~JvmTIAgent() throw(AgentException);

	void Init(JavaVM *vm) const throw(AgentException);

	void ParseOptions(const char* str) const throw(AgentException);

	void AddCapability() const throw(AgentException);

	void RegisterEvent() const throw(AgentException);

	static void JNICALL HandleMethodEntry(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jmethodID method);

	static void JNICALL HandleClassFileLoadHook(
		jvmtiEnv *jvmti_env,
		JNIEnv* jni_env,
		jclass class_being_redefined,
		jobject loader,
		const char* name,
		jobject protection_domain,
		jint class_data_len,
		const unsigned char* class_data,
		jint* new_class_data_len,
		unsigned char** new_class_data);

private:
	static void CheckException(jvmtiError error) throw(AgentException)
	{
		// 可以根据错误类型扩展对应的异常，这里只做简单处理
		if (error != JVMTI_ERROR_NONE) {
			throw AgentException(error);
		}
	}

	static jvmtiEnv * m_jvmti;
	static char* m_options;
};
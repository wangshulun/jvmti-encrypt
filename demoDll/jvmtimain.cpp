#include <iostream>

#include "jvmtiagent.h"
#include "jvmti.h"

using namespace std;

// ��JVM Tool Interface 1.2�� https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html
// ��IBM JVMTI �� Agentʵ�֡� https://www.ibm.com/developerworks/cn/java/j-lo-jpda2/index.html

/*
Agent �Ĺ�������:
Agent ���� Java ���������֮ʱ���صģ�������ش����������ʼ�������ڣ������ʱ����ϣ�
	���е� Java �඼δ����ʼ����
	���еĶ���ʵ����δ��������
	�����û���κ� Java ���뱻ִ�У�
*/

/*
Agent ��ں���
	@param JavaVM *vm
		�����������һ�� JavaVM ָ�루������ȡjvmtiEnv ��ָ��,��� JVMTI ������ʹ�����������Լ������еĲ�����
		jvmtiEnv *jvmti;
		(*jvm)->GetEnv(jvm, &jvmti, JVMTI_VERSION_1_0);

	@param char *options
		�����в���
*/
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
	cout << "[JVMTI Agent] Agent_OnLoad ��ʼ(" << vm << ")" << endl;
	try {
		JvmTIAgent* agent = new JvmTIAgent();
		agent->Init(vm);
		cout << "[JVMTI Agent] Agent_OnLoad init���(" << vm << ")" << endl;
		agent->ParseOptions(options);
		cout << "[JVMTI Agent] Agent_OnLoad ParseOptions���(" << vm << ")" << endl;
		agent->AddCapability();
		cout << "[JVMTI Agent] Agent_OnLoad AddCapability���(" << vm << ")" << endl;
		agent->RegisterEvent();
		cout << "[JVMTI Agent] Agent_OnLoad RegisterEvent���(" << vm << ")" << endl;

	}
	catch (AgentException& e) {
		cout << "[JVMTI Agent] Error ����, " << e.what() << " [" << e.ErrCode() << "]" << endl;
		return JNI_ERR;
	}

	return JNI_OK;
}

/*
Agent ������ɣ�ж��
*/
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
	cout << "[JVMTI Agent] Agent_OnUnload agent�������(" << vm << ")" << endl;
}
#include <iostream>

#include "jvmtiagent.h"
#include "jvmti.h"

using namespace std;

// 《JVM Tool Interface 1.2》 https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html
// 《IBM JVMTI 和 Agent实现》 https://www.ibm.com/developerworks/cn/java/j-lo-jpda2/index.html

/*
Agent 的工作过程:
Agent 是在 Java 虚拟机启动之时加载的，这个加载处于虚拟机初始化的早期，在这个时间点上：
	所有的 Java 类都未被初始化；
	所有的对象实例都未被创建；
	因而，没有任何 Java 代码被执行；
*/

/*
Agent 入口函数
	@param JavaVM *vm
		虚拟机传入了一个 JavaVM 指针（用来获取jvmtiEnv 的指针,获得 JVMTI 函数的使用能力），以及命令行的参数。
		jvmtiEnv *jvmti;
		(*jvm)->GetEnv(jvm, &jvmti, JVMTI_VERSION_1_0);

	@param char *options
		命令行参数
*/
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
	cout << "[JVMTI Agent] Agent_OnLoad 开始(" << vm << ")" << endl;
	try {
		JvmTIAgent* agent = new JvmTIAgent();
		agent->Init(vm);
		cout << "[JVMTI Agent] Agent_OnLoad init完成(" << vm << ")" << endl;
		agent->ParseOptions(options);
		cout << "[JVMTI Agent] Agent_OnLoad ParseOptions完成(" << vm << ")" << endl;
		agent->AddCapability();
		cout << "[JVMTI Agent] Agent_OnLoad AddCapability完成(" << vm << ")" << endl;
		agent->RegisterEvent();
		cout << "[JVMTI Agent] Agent_OnLoad RegisterEvent完成(" << vm << ")" << endl;

	}
	catch (AgentException& e) {
		cout << "[JVMTI Agent] Error 错误, " << e.what() << " [" << e.ErrCode() << "]" << endl;
		return JNI_ERR;
	}

	return JNI_OK;
}

/*
Agent 任务完成，卸载
*/
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
	cout << "[JVMTI Agent] Agent_OnUnload agent任务完成(" << vm << ")" << endl;
}
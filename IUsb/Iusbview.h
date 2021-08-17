// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� IUSBVIEW_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// IUSBVIEW_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef IUSBVIEW_EXPORTS
#define IUSBVIEW_API __declspec(dllexport)
#else
#define IUSBVIEW_API __declspec(dllimport)
#endif

// �����Ǵ� Iusbview.dll ������
class IUSBVIEW_API CIusbview {
public:
	CIusbview(void);
	// TODO: �ڴ�������ķ�����

	BOOL IsUsbExists(LPCTSTR lpVid, LPCTSTR lpPid, LPCTSTR lpSerial);
	BOOL GetSerialNumber(LPCTSTR lpPath, TCHAR* pOut);
};

extern IUSBVIEW_API int nIusbview;

IUSBVIEW_API int fnIusbview(void);

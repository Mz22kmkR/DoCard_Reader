#include <stdio.h>

#include "felicalib.h"

static void suica_dump_history(uint8 *data);
static const char *consoleType(int ctype);
static const char *procType(int proc);
static int read4b(uint8 *p);
static int read2b(uint8 *p);

// �T�[�r�X�R�[�h
#define SERVICE_SUICA_INOUT     0x108f
#define SERVICE_DOCARD_HISTORY   0x010f	//90
#define SERVICE_DOCARD_BALANCE	0x008b


int _tmain(int argc, _TCHAR *argv[])
{
	pasori *p;
	felica *f;
	int i;
	uint8 data[16];
	uint8 balance[16];

	p = pasori_open(NULL);
	if (!p) {
		fprintf(stderr, "PaSoRi open failed.\n");
		exit(1);
	}
	pasori_init(p);

	f = felica_polling(p, 0x8db6, 0, 0);	//POLLING_SUICA
	if (!f) {
		fprintf(stderr, "Polling card failed.\n");
		exit(1);
	}

	printf("IDm: ");
	for (i = 0; i < 8; i++) {
		printf("%02x", f->IDm[i]);
	}
	printf("\n");

	felica_read_without_encryption02(f, 0x008b, 0, (uint8)i, data);

	for (i = 0; ; i++) {
		if (felica_read_without_encryption02(f, SERVICE_DOCARD_HISTORY, 0, (uint8)i, data))
		{
			break;
		}

		suica_dump_history(data);
		
	}

	felica_free(f);
	pasori_close(p);

	return 0;
}

static void suica_dump_history(uint8 *data)
{
	int proc, date, time, balance, seq, region, use;
	int in_line, in_sta, out_line, out_sta;
	int yy, mm, dd;

	proc = data[12];             // ����
	date = read2b(data + 4);    // ���t
	balance = read2b(data + 14);// �c��
	//balance = N2HS(balance);
	use = read2b(data + 9);//[9],[10]

	seq = data[11];
	//region = seq & 0xff;        // Region
	//seq >>= 8;                  // �A��

	out_line = -1;
	out_sta = -1;
	time = -1;

	//�ʂ��ԍ�(FF�ȍ~�͓�)
	printf("�A��:%d ", seq);

	// ���t
	yy = date >> 7;	//7
	mm = data[0];
	dd = data[1];
	printf("%02d/%02d/%02d ", yy, mm, dd);

	printf("����:%s ", procType(proc));



	// ����
	if (time > 0) {
		int hh = time >> 11;
		int min = (time >> 5) & 0x3f;

		printf(" %02d:%02d ", hh, min);
	}
	if (proc == 0x02)
	{
	printf("�g�p���z:%d�~ ", use);
	}
	else
	{
		int chage = read2b(data + 8);
		printf("�`���[�W���z:%d�~ ", chage);
	}
	printf("�c��:%d�~ ", balance);
	printf("\n");
}

static const char *consoleType(int ctype)
{
	switch (ctype) {
	case 0x03: return "���Z�@";
	case 0x05: return "�ԍڒ[��";
	case 0x08: return "�����@";
	case 0x0b: return "�ԍڒ[��";
	case 0x12: return "�����@";
	case 0x16: return "���D�@";
	case 0x17: return "�ȈՉ��D�@";
	case 0x18: return "�����[��";
	case 0x1a: return "���D�[��";
	case 0x1b: return "�g�ѓd�b";
	case 0x1c: return "��p���Z�@";
	case 0x1d: return "�A�����D�@";
	case 0xc7: return "����";
	case 0xc8: return "���̋@";
	}
	return "???";
}

static const char *procType(int proc)
{
	switch (proc) {
	case 0x01: return "�^���x��";
	case 0x02: return "�^���x��";
	case 0x03: return "���w";
	case 0x04: return "���Z";
	case 0x07: return "������X�V";
	case 0x15: return "�`���[�W";
	case 0x09: return "�^���x��";
	case 0x2d: return "������X�V";
	/*case 0x0d: return "�o�X";
	case 0x0f: return "�o�X";*/
	case 0x14: return "�V�K���s";
	/*case 0x46: return "����";
	case 0x49: return "����";
	case 0xc6: return "����(�������p)";*/
	}
	return "???";
}

static int read4b(uint8 *p)
{
	int v;
	v = (*p++) << 24;
	v |= (*p++) << 16;
	v |= (*p++) << 8;
	v |= *p;
	return v;
}

static int read2b(uint8 *p)
{
	int v;
	v = (*p++) << 8;
	v |= *p;
	return v;
}



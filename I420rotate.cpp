/**
 * 最简单的基于FFmpeg的视频播放器2(SDL升级版)
 * Simplest FFmpeg Player 2(SDL Update)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * 本版本中使用SDL消息机制刷新视频画面。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 * 备注:
 * 标准版在播放视频的时候，画面显示使用延时40ms的方式。这么做有两个后果：
 * （1）SDL弹出的窗口无法移动，一直显示是忙碌状态
 * （2）画面显示并不是严格的40ms一帧，因为还没有考虑解码的时间。
 * SU（SDL Update）版在视频解码的过程中，不再使用延时40ms的方式，而是创建了
 * 一个线程，每隔40ms发送一个自定义的消息，告知主函数进行解码显示。这样做之后：
 * （1）SDL弹出的窗口可以移动了
 * （2）画面显示是严格的40ms一帧
 * Remark:
 * Standard Version use's SDL_Delay() to control video's frame rate, it has 2
 * disadvantages:
 * (1)SDL's Screen can't be moved and always "Busy".
 * (2)Frame rate can't be accurate because it doesn't consider the time consumed
 * by avcodec_decode_video2()
 * SU（SDL Update）Version solved 2 problems above. It create a thread to send SDL
 * Event every 40ms to tell the main loop to decode and show video frames.
 */


#include <iostream>

enum Degree {kRotate90, kRotate180, kRotate270, kRotateMirror};
// 旋转
void RotateI420(uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
	uint8_t* dst_y, uint8_t* dst_u, uint8_t* dst_v,
	uint32_t width, uint32_t height, int degree);
void Rotate90_I420(uint8_t* src, uint8_t* dst,uint32_t width, uint32_t height);
void Rotate180_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height);
void Rotate270_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height);
void RotateMirror_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height);

int main()
{
	FILE* file_open = nullptr;
	FILE* out_file_open = nullptr;
	fopen_s(&file_open,"bigbuckbunny_480x272.yuv", "rb+");
	fopen_s(&out_file_open, "bigbuckbunny_272x480.yuv", "wb+");
	int w = 480, h = 272;
	int buffLen = 480 * 272 * 3 / 2;
	char* inbuff = new char[buffLen];
	char* outbuff = new char[buffLen];
	while (true)
	{
		if (fread(inbuff,1, buffLen,file_open)!= buffLen)
		{
			break;
		}
		RotateMirror_I420((uint8_t*)inbuff, (uint8_t*)outbuff, w, h);
		fwrite(outbuff, 1, buffLen, out_file_open);
	}
	if (inbuff)
	{
		delete inbuff;
		inbuff = nullptr;
	}
	
    std::cout << "Hello World!\n";
}

void RotateI420(uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
	uint8_t* dst_y, uint8_t* dst_u, uint8_t* dst_v,
	uint32_t width, uint32_t height, int degree) {
	if (degree == kRotate90) {
		// 第一步我们只转Y
	// 旋转90度
		int index = 0;
		for (int i = 0; i < width; ++i) {
			for (int j = height - 1; j >= 0; --j) {
				dst_y[index++] = src_y[j * width + i];
			}
		}

		int uv_index = 0;
		for (int i = 0; i < width / 2; i++) {
			for (int j = height / 2 - 1; j >= 0; j--)
			{
				dst_u[uv_index] = src_u[width / 2 * j + i];
				dst_v[uv_index] = src_v[width / 2 * j + i];
				uv_index++;
			}
		}
	}

	if (degree == kRotate180) {
		// 第一步我们只转Y
	// 旋转180度
		int index = 0;
		for (int i = width * height-1; i >= 0 ; --i) {
			dst_y[index++] = src_y[i];
		}

		int uv_index = 0;
		for (int i = width / 2 * height / 2; i >= 0; i--) {
			dst_u[uv_index] = src_u[i];
			dst_v[uv_index] = src_v[i];
			uv_index++;
		}
	}

	if (degree == kRotate270) {
		// 第一步我们只转Y
	// 旋转270度
		int index = 0;
		for (int i = width-1; i >= 0; --i) {
			for (int j = 0; j < height; ++j) {
				dst_y[index++] = src_y[j * width+i];
			}
		}

		int uv_index = 0;
		for (int i = width / 2-1; i >= 0; i--) {
			for (int j =  0; j< height / 2; j++)
			{
				dst_u[uv_index] = src_u[width / 2 * j+i];
				dst_v[uv_index] = src_v[width / 2 * j+ i];
				uv_index++;
			}
		}
	}

	if (degree == kRotateMirror) {
		// 第一步我们只转Y
	// 镜像
		for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; i++)
			{
				dst_y[j * width + width-1-i] = src_y[j * width + i];
			}
		}

		for (int j = 0; j < height / 2; ++j) {
			for (int i = 0; i < width / 2; i++)
			{
				dst_u[j * width / 2 + width / 2-1 - i] = src_u[j * width / 2 + i];
				dst_v[j * width / 2 + width / 2-1 - i] = src_v[j * width / 2 + i];
			}
		}
	}

}
void Rotate90_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height)
{
	uint8_t* src_y = src;
	uint8_t* src_u = src+ width* height;
	uint8_t* src_v = src + width * height + width * height/4;
	uint8_t* dst_y = dst ;
	uint8_t* dst_u = dst + width * height;
	uint8_t* dst_v = dst + width * height + width * height / 4;

	RotateI420(src_y, src_u, src_v,
		       dst_y, dst_u, dst_v,
		       width, height, kRotate90);
}

void Rotate180_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height)
{
	uint8_t* src_y = src;
	uint8_t* src_u = src + width * height;
	uint8_t* src_v = src + width * height + width * height / 4;
	uint8_t* dst_y = dst;
	uint8_t* dst_u = dst + width * height;
	uint8_t* dst_v = dst + width * height + width * height / 4;

	RotateI420(src_y, src_u, src_v,
		dst_y, dst_u, dst_v,
		width, height, kRotate180);
}

void Rotate270_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height)
{
	uint8_t* src_y = src;
	uint8_t* src_u = src + width * height;
	uint8_t* src_v = src + width * height + width * height / 4;
	uint8_t* dst_y = dst;
	uint8_t* dst_u = dst + width * height;
	uint8_t* dst_v = dst + width * height + width * height / 4;

	RotateI420(src_y, src_u, src_v,
		dst_y, dst_u, dst_v,
		width, height, kRotate270);
}

void RotateMirror_I420(uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height)
{
	uint8_t* src_y = src;
	uint8_t* src_u = src + width * height;
	uint8_t* src_v = src + width * height + width * height / 4;
	uint8_t* dst_y = dst;
	uint8_t* dst_u = dst + width * height;
	uint8_t* dst_v = dst + width * height + width * height / 4;

	RotateI420(src_y, src_u, src_v,
		dst_y, dst_u, dst_v,
		width, height, kRotateMirror);
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

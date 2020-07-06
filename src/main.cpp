#include "main_header.hpp"

int main()
{
	// get process id

	g_process_id = Edmapper::GetProcessID("notepad.exe");

	if (g_process_id == 0) {
		std::printf("[-]Couldn't get process ID\n");
		return -1;
	}


	// open handle to process

	gProc_handle = Edmapper::OpenProcessHandle(g_process_id);

	// add check for handle here.

	// read dll

	if (Edmapper::GetRawDataFromFile("C:\\Users\\User\\Desktop\\cpp-projects\\EDMapper\\x64\\Release\\test.dll") != false)
	{
		std::cout << "read dll." << '\n';
	}

	// validate image

	if (Edmapper::IsValidImage())
	{
		std::cout << "Image is valid." << '\n';
	}

	// allocate local image 
	const auto image_size = pOldnt_headers->OptionalHeader.SizeOfImage;
	void* l_image = nullptr;
	l_image = VirtualAlloc(nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	if (!l_image)
	{
		delete[] rawDll_data;
		return -1;
	}

	// copy all headers from our dll image.
	std::memcpy(l_image, rawDll_data,pOldnt_headers->OptionalHeader.SizeOfHeaders);


	// copy sections into local image.
	Edmapper::CopyImageSections(l_image,pOldnt_headers);


	// fix imports
	if (!Edmapper::FixImageImports(l_image, pOldnt_headers))
	{
		std::cerr << "[ERROR] couldn't fix image imports" << '\n';
		delete[] rawDll_data;
		VirtualFree(l_image, 0, MEM_RELEASE);
		return -1;
	}
		
	// allocate image in target process
	const auto image_base = pOldnt_headers->OptionalHeader.ImageBase;
	void* m_image = nullptr;
	// first try to allocate at prefered load address
	m_image = VirtualAllocEx(gProc_handle.get(), reinterpret_cast<LPVOID>(image_base), image_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!m_image)
	{
		// if we couldn't allocate at prefered address then just allocate memory on any random place in memory 
		// but we will need to fix relocation of image
		m_image = VirtualAllocEx(gProc_handle.get(), nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		// if faild to allocate then cleanup & return
		if (!m_image)
		{
			delete[] rawDll_data;
			VirtualFree(l_image, 0, MEM_RELEASE); // we need todo this whenever we fail
			std::cerr << "[ERROR] couldn't allocate memory in target process." << '\n';
			return -1;
		}

		// fix relocation
		Edmapper::FixImageRelocations(m_image, pOldnt_headers);

	}

	// no need to fix relocations since we loaded at prefered base address.

	// write content of our dll aka local image into the allocated memory in target process


	// make an option to check TLS callbacks section if its valid or not before relocation

	// call shellcode

	std::printf("Everything worked.\n");
	// free mapped image?? why lol iwant to understand this
	VirtualFree(l_image, 0, MEM_RELEASE);
	delete[] rawDll_data;

	std::cin.get();
}
#include "InliteCOMClient.h"

std::vector<Barcode> InliteCOMClient::post_image(const unsigned char* image_path) {
	std::vector<Barcode> result;

	ICiQRPtr reader = _server->CreateQR();
	reader->Image->Open(_bstr_t(image_path), 1);
	reader->Find(0);
	
	for (int i = 1; i <= reader->Barcodes->Count; i ++) {
		auto bc = reader->Barcodes->Item[i];
		std::string text(bc->Text);
		std::wstring w_text;
		w_text.assign(text.begin(), text.end());
		auto m_bc = Barcode(bc->Rect->left, bc->Rect->right, bc->Rect->top, bc->Rect->bottom, w_text);
		result.push_back(m_bc);
	}
	return result;
}


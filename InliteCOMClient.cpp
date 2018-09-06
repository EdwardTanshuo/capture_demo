#include "InliteCOMClient.h"

std::vector<Barcode> InliteCOMClient::post_image(const char* image_path) {
    ICiQRPtr reader = _server->CreateQR();
    reader->Image->Open(bstr_t(image_path), 1);
    reader->Find(0);

    std::vector<Barcode> barcodes;
    int i;
    for (i = 1; i <= reader->Barcodes->Count; i++) {
        auto bc = reader->Barcodes->Item[i];
        std::string text(bc->Text);
        std::wstring w_text;
        w_text.assign(text.begin(), text.end());
        auto m_bc = Barcode(bc->Rect->left, bc->Rect->right, bc->Rect->top, bc->Rect->bottom, w_text);
        barcodes.push_back(m_bc);
    }
    if (barcodes.size() == 0) {
        return std::vector<Barcode>();
    }

    return barcodes;
}


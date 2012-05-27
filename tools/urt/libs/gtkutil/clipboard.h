
#if !defined(INCLUDED_GTKUTIL_CLIPBOARD_H)
#define INCLUDED_GTKUTIL_CLIPBOARD_H

class TextOutputStream;
typedef void(*ClipboardCopyFunc)(TextOutputStream&);
void clipboard_copy(ClipboardCopyFunc copy);

class TextInputStream;
typedef void(*ClipboardPasteFunc)(TextInputStream&);
void clipboard_paste(ClipboardPasteFunc paste);

#endif

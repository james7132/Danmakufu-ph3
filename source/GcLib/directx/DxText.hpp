#ifndef __DIRECTX_DXTEXT__
#define __DIRECTX_DXTEXT__

#include"DxConstant.hpp"
#include"Texture.hpp"
#include"RenderObject.hpp"

namespace directx
{
	class DxChar;
	class DxCharCache;
	class DxCharCacheKey;
	class DxTextRenderer;
	class DxText;

	/**********************************************************
	//DxFont
	**********************************************************/
	class DxFont
	{
		friend DxCharCache;
		friend DxCharCacheKey;
		public:
			enum
			{
				BORDER_NONE,
				BORDER_FULL,
				BORDER_SHADOW,
			};
		protected:
			LOGFONT info_;//�t�H���g���
			D3DCOLOR colorTop_;
			D3DCOLOR colorBottom_;
			int typeBorder_;//�����
			int widthBorder_;
			D3DCOLOR colorBorder_;//�����F
		public:
			DxFont();
			virtual ~DxFont();
			void SetLogFont(LOGFONT& font){info_ = font;}
			LOGFONT GetLogFont(){return info_;}
			void SetTopColor(D3DCOLOR color){colorTop_ = color;}
			D3DCOLOR GetTopColor(){return colorTop_;}
			void SetBottomColor(D3DCOLOR color){colorBottom_ = color;}
			D3DCOLOR GetBottomColor(){return colorBottom_;}
			void SetBorderType(int type){typeBorder_ = type;}
			int GetBorderType(){return typeBorder_;}
			void SetBorderWidth(int width){widthBorder_ = width;}
			int GetBorderWidth(){return widthBorder_;}
			void SetBorderColor(D3DCOLOR color){colorBorder_ = color;}
			D3DCOLOR GetBorderColor(){return colorBorder_;}
	};

	/**********************************************************
	//DxChar
	//����1�����̃e�N�X�`��
	**********************************************************/
	class DxChar
	{
			gstd::ref_count_ptr<Texture> texture_;
			int code_;
			DxFont font_;

			int width_;
			int height_;
		public:
			DxChar();
			virtual ~DxChar();
			bool Create(int code, gstd::Font& winFont, DxFont& dxFont);
			gstd::ref_count_ptr<Texture> GetTexture(){return texture_;}
			int GetWidth(){return width_;}
			int GetHeight(){return height_;}
			LOGFONT GetInfo(){return font_.GetLogFont();}
	};


	/**********************************************************
	//DxCharCache
	//�����L���b�V��
	**********************************************************/
	class DxCharCacheKey
	{
		friend DxCharCache;
		friend DxTextRenderer;
		private:
			int code_;
			DxFont font_;
		public:
			bool operator ==(const DxCharCacheKey& key)const
			{
				bool res = true;
				res &= (code_ == key.code_);
				res &= (font_.colorTop_ == key.font_.colorTop_);
				res &= (font_.colorBottom_ == key.font_.colorBottom_);
				res &= (font_.typeBorder_ == key.font_.typeBorder_);
				res &= (font_.widthBorder_ == key.font_.widthBorder_);
				res &= (font_.colorBorder_ == key.font_.colorBorder_);
				if(!res)return res;
				res &= (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) == 0);
				return res;
			}
			bool operator<(const DxCharCacheKey& key)const
			{
				if(code_ != key.code_)return code_ < key.code_;
				if(font_.colorTop_ != key.font_.colorTop_)return font_.colorTop_ < key.font_.colorTop_;
				if(font_.colorBottom_ != key.font_.colorBottom_)return font_.colorBottom_ < key.font_.colorBottom_;
//				if(font_.typeBorder_ != key.font_.typeBorder_)return (font_.typeBorder_ != key.font_.typeBorder_ );
				if(font_.widthBorder_ != key.font_.widthBorder_)return font_.widthBorder_ < key.font_.widthBorder_;
				if(font_.colorBorder_ != key.font_.colorBorder_)return font_.colorBorder_ < key.font_.colorBorder_;
				return (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) < 0);
			}
	};
	class DxCharCache 
	{
		friend DxTextRenderer;
		private:
			int sizeMax_;
			int countPri_;
			std::map<DxCharCacheKey, gstd::ref_count_ptr<DxChar> > mapCache_;
			std::map<int, DxCharCacheKey > mapPriKey_;
			std::map<DxCharCacheKey, int > mapKeyPri_;

			void _arrange();
		public:
			DxCharCache();
			~DxCharCache();
			void Clear();
			int GetCacheCount(){return mapCache_.size();}

			gstd::ref_count_ptr<DxChar> GetChar(DxCharCacheKey& key);
			void AddChar(DxCharCacheKey& key, gstd::ref_count_ptr<DxChar> value);
	};

	/**********************************************************
	//DxTextScanner
	**********************************************************/
	class DxTextScanner;
	class DxTextToken
	{
		friend DxTextScanner;
		public:
			enum Type
			{
				TK_UNKNOWN,TK_EOF,TK_NEWLINE,
				TK_ID,
				TK_INT,TK_REAL,TK_STRING,

				TK_OPENP,TK_CLOSEP,TK_OPENB,TK_CLOSEB,TK_OPENC,TK_CLOSEC,
				TK_SHARP,

				TK_COMMA,TK_EQUAL,
				TK_ASTERISK,TK_SLASH,TK_COLON,TK_SEMICOLON,TK_TILDE,TK_EXCLAMATION,
				TK_PLUS,TK_MINUS,
				TK_LESS,TK_GREATER,

				TK_TEXT,
			};
		protected:
			Type type_;
			std::wstring element_;
		public:
			DxTextToken(){type_ = TK_UNKNOWN;}
			DxTextToken(Type type, std::wstring element){type_ = type; element_ = element;}

			virtual ~DxTextToken(){}
			Type GetType(){return type_;}
			std::wstring& GetElement(){return element_;}

			int GetInteger();
			double GetReal();
			bool GetBoolean();
			std::wstring GetString();
			std::wstring& GetIdentifier();
	};

	class DxTextScanner
	{
		public:
			const static int TOKEN_TAG_START;
			const static int TOKEN_TAG_END;
			const static std::wstring TAG_START;
			const static std::wstring TAG_END;
			const static std::wstring TAG_NEW_LINE;
			const static std::wstring TAG_RUBY;
			const static std::wstring TAG_FONT;
		protected:
			int line_;
			std::vector<wchar_t> buffer_;
			std::vector<wchar_t>::iterator pointer_;//���̈ʒu
			DxTextToken token_;//���݂̃g�[�N��
			boolean bTagScan_;

			wchar_t _NextChar();//�|�C���^��i�߂Ď��̕����𒲂ׂ�
			virtual void _SkipComment();//�R�����g���Ƃ΂�
			virtual void _SkipSpace();//�󔒂��Ƃ΂�
			virtual void _RaiseError(std::wstring str);//��O�𓊂��܂�
			bool _IsTextStartSign();
			bool _IsTextScan();
		public:
			DxTextScanner(wchar_t* str,int charCount);
			DxTextScanner(std::wstring str);
			DxTextScanner(std::vector<wchar_t>& buf);
			virtual ~DxTextScanner();

			DxTextToken& GetToken();//���݂̃g�[�N�����擾
			DxTextToken& Next();
			bool HasNext();
			void CheckType(DxTextToken& tok, int type);
			void CheckIdentifer(DxTextToken& tok, std::wstring id);
			int GetCurrentLine();
			int SearchCurrentLine();

			std::vector<wchar_t>::iterator GetCurrentPointer();
			void SetCurrentPointer(std::vector<wchar_t>::iterator pos);
			void SetPointerBegin(){pointer_ = buffer_.begin();}
			int GetCurrentPosition();
			void SetTagScanEnable(bool bEnable){bTagScan_ = bEnable;}
	};

	/**********************************************************
	//DxTextRenderer
	//�e�L�X�g�`��G���W��
	**********************************************************/
	class DxTextLine;
	class DxTextInfo;
	class DxTextRenderer;
	class DxTextTag;
	class DxTextTag
	{
		public:
			enum
			{
				TYPE_UNKNOWN,
				TYPE_RUBY,
				TYPE_FONT
			};
		protected:
			int typeTag_;
			int indexTag_;
		public:
			DxTextTag(){indexTag_ = 0;typeTag_=TYPE_UNKNOWN;}
			virtual ~DxTextTag(){};
			int GetTagType(){return typeTag_;}
			int GetTagIndex(){return indexTag_;}
			void SetTagIndex(int index){indexTag_ = index;}
	};
	class DxTextTag_Ruby : public DxTextTag
	{
			int leftMargin_;
			std::wstring text_;
			std::wstring ruby_;
			gstd::ref_count_ptr<DxText> dxText_;
		public:
			DxTextTag_Ruby(){typeTag_ = TYPE_RUBY;leftMargin_ = 0;}
			int GetLeftMargin(){return leftMargin_;}
			void SetLeftMargin(int left){leftMargin_ = left;}

			std::wstring& GetText(){return text_;}
			void SetText(std::wstring text){text_ = text;}
			std::wstring& GetRuby(){return ruby_;}
			void SetRuby(std::wstring ruby){ruby_ = ruby;}

			gstd::ref_count_ptr<DxText> GetRenderText(){return dxText_;}
			void SetRenderText(gstd::ref_count_ptr<DxText> text){dxText_ = text;}
	};
	class DxTextTag_Font : public DxTextTag
	{
			DxFont font_;
		public:
			DxTextTag_Font(){typeTag_ = TYPE_FONT;}
			void SetFont(DxFont font){font_ = font;}
			DxFont& GetFont(){return font_;}
	};
	class DxTextLine
	{
		friend DxTextRenderer;
		protected:
			int width_;
			int height_;
			int sidePitch_;
			std::vector<int> code_;
			std::vector<gstd::ref_count_ptr<DxTextTag> > tag_;
		public:
			DxTextLine(){width_=0;height_=0;sidePitch_=0;}
			virtual ~DxTextLine(){};
			int GetWidth(){return width_;}
			int GetHeight(){return height_;}
			int GetSidePitch(){return sidePitch_;}
			void SetSidePitch(int pitch){sidePitch_ = pitch;}
			std::vector<int>& GetTextCodes(){return code_;}
			int GetTextCodeCount(){return code_.size();}
			int GetTagCount(){return tag_.size();}
			gstd::ref_count_ptr<DxTextTag> GetTag(int index){return tag_[index];}
	};
	class DxTextInfo
	{
		friend DxTextRenderer;
		protected:
			int totalWidth_;
			int totalHeight_;
			int lineValidStart_;
			int lineValidEnd_;
			bool bAutoIndent_;
			std::vector<gstd::ref_count_ptr<DxTextLine> > textLine_;
		public:
			DxTextInfo(){totalWidth_=0;totalHeight_=0;lineValidStart_=1;lineValidEnd_=0;bAutoIndent_=false;}
			virtual ~DxTextInfo(){};
			int GetTotalWidth(){return totalWidth_;}
			int GetTotalHeight(){return totalHeight_;}
			int GetValidStartLine(){return lineValidStart_;}
			int GetValidEndLine(){return lineValidEnd_;}
			void SetValidStartLine(int line){lineValidStart_=line;}
			void SetValidEndLine(int line){lineValidEnd_=line;}
			bool IsAutoIndent(){return bAutoIndent_;}
			void SetAutoIndent(bool bEnable){bAutoIndent_ = bEnable;}

			int GetLineCount(){return textLine_.size();}
			void AddTextLine(gstd::ref_count_ptr<DxTextLine> text){textLine_.push_back(text), lineValidEnd_++;}
			gstd::ref_count_ptr<DxTextLine> GetTextLine(int pos){return textLine_[pos];}

	};

	class DxTextRenderObject
	{
		struct ObjectData
		{
			POINT bias;
			gstd::ref_count_ptr<Sprite2D> sprite;
		};
		protected:
			POINT position_;//�ړ�����W
			D3DXVECTOR3 angle_;//��]�p�x
			D3DXVECTOR3 scale_;//�g�嗦
			D3DCOLOR color_;
			std::list<ObjectData> listData_;
			D3DXVECTOR2 center_;//���W�ϊ��̒��S
			bool bAutoCenter_;
			bool bPermitCamera_;
			gstd::ref_count_ptr<Shader> shader_;

		public:
			DxTextRenderObject();
			virtual ~DxTextRenderObject(){}

			void Render();
			void AddRenderObject(gstd::ref_count_ptr<Sprite2D> obj);
			void AddRenderObject(gstd::ref_count_ptr<DxTextRenderObject> obj, POINT bias);

			POINT GetPosition(){return position_;}
			void SetPosition(POINT pos){position_.x = pos.x; position_.y = pos.y;}
			void SetPosition(int x, int y){position_.x = x; position_.y = y;}
			void SetVertexColor(D3DCOLOR color){color_ = color;}

			void SetAngle(D3DXVECTOR3& angle){angle_ = angle;}
			void SetScale(D3DXVECTOR3& scale){scale_ = scale;}
			void SetTransCenter(D3DXVECTOR2& center){center_ = center;}
			void SetAutoCenter(bool bAuto){bAutoCenter_ = bAuto;}
			void SetPermitCamera(bool bPermit){bPermitCamera_ = bPermit;}

			gstd::ref_count_ptr<Shader> GetShader(){return shader_;}
			void SetShader(gstd::ref_count_ptr<Shader> shader){shader_ = shader;}
	};

	class DxTextRenderer
	{	
		static DxTextRenderer* thisBase_;
		public:
			enum
			{
				ALIGNMENT_LEFT,
				ALIGNMENT_RIGHT,
				ALIGNMENT_TOP,
				ALIGNMENT_BOTTOM,
				ALIGNMENT_CENTER,
			};

		protected:
			DxCharCache cache_;
			gstd::Font winFont_;
			D3DCOLOR colorVertex_;
			gstd::CriticalSection lock_;

			SIZE _GetTextSize(HDC hDC, wchar_t* pText);
			gstd::ref_count_ptr<DxTextLine> _GetTextInfoSub(std::wstring text, DxText* dxText, DxTextInfo* textInfo, gstd::ref_count_ptr<DxTextLine> textLine, HDC& hDC, int& totalWidth, int &totalHeight);
			void _CreateRenderObject(gstd::ref_count_ptr<DxTextRenderObject> objRender, POINT pos, DxFont& dxFont, gstd::ref_count_ptr<DxTextLine> textLine);
			std::wstring _ReplaceRenderText(std::wstring& text);
		public:
			DxTextRenderer();
			virtual ~DxTextRenderer();
			static DxTextRenderer* GetBase(){return thisBase_;}
			bool Initialize();
			gstd::CriticalSection& GetLock(){return lock_;}

			void ClearCache(){cache_.Clear();}
			void SetFont(LOGFONT& logFont){winFont_.CreateFontIndirect(logFont);}
			void SetVertexColor(D3DCOLOR color){colorVertex_ = color;}
			gstd::ref_count_ptr<DxTextInfo> GetTextInfo(DxText* dxText);

			gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject(DxText* dxText, gstd::ref_count_ptr<DxTextInfo> textInfo);

			void Render(DxText* dxText);
			void Render(DxText* dxText, gstd::ref_count_ptr<DxTextInfo> textInfo);

			int GetCacheCount(){return cache_.GetCacheCount();}

			bool AddFontFromFile(std::wstring path);
	};

	/**********************************************************
	//DxText
	//�e�L�X�g�`��
	**********************************************************/
	class DxText
	{
		friend DxTextRenderer;
		public:
			enum
			{
				ALIGNMENT_LEFT = DxTextRenderer::ALIGNMENT_LEFT,
				ALIGNMENT_RIGHT = DxTextRenderer::ALIGNMENT_RIGHT,
				ALIGNMENT_TOP = DxTextRenderer::ALIGNMENT_TOP,
				ALIGNMENT_BOTTOM = DxTextRenderer::ALIGNMENT_BOTTOM,
				ALIGNMENT_CENTER = DxTextRenderer::ALIGNMENT_CENTER,
			};
		protected:
			DxFont dxFont_;
			POINT pos_;
			int widthMax_;
			int heightMax_;
			int sidePitch_;
			int linePitch_;
			RECT margin_;
			int alignmentHorizontal_;
			int alignmentVertical_;
			D3DCOLOR colorVertex_;
			bool bPermitCamera_;
			bool bSyntacticAnalysis_;

			gstd::ref_count_ptr<Shader> shader_;
			std::wstring text_;
		public:
			DxText();
			virtual ~DxText();
			void Copy(const DxText& src);
			virtual void Render();
			void Render(gstd::ref_count_ptr<DxTextInfo> textInfo);

			gstd::ref_count_ptr<DxTextInfo> GetTextInfo();
			gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject();
			gstd::ref_count_ptr<DxTextRenderObject> CreateRenderObject(gstd::ref_count_ptr<DxTextInfo> textInfo);

			DxFont GetFont(){return dxFont_;}
			void SetFont(DxFont font){dxFont_ = font;}
			void SetFont(LOGFONT logFont){dxFont_.SetLogFont(logFont);}

			void SetFontType(const wchar_t* type);
			int GetFontSize(){return dxFont_.GetLogFont().lfHeight;}
			void SetFontSize(int size){LOGFONT info=dxFont_.GetLogFont();info.lfHeight=size;SetFont(info);}
			void SetFontBold(bool bBold){LOGFONT info=dxFont_.GetLogFont();info.lfWeight=(bBold==false)*FW_NORMAL+(bBold==TRUE)*FW_BOLD;SetFont(info);}
			void SetFontItalic(bool bItalic){LOGFONT info=dxFont_.GetLogFont();info.lfItalic = bItalic;SetFont(info);}
			void SetFontUnderLine(bool bLine){LOGFONT info=dxFont_.GetLogFont();info.lfUnderline = bLine;SetFont(info);}
			
			void SetFontColorTop(D3DCOLOR color){dxFont_.SetTopColor(color);}
			void SetFontColorBottom(D3DCOLOR color){dxFont_.SetBottomColor(color);}
			void SetFontBorderWidth(int width){dxFont_.SetBorderWidth(width);}
			void SetFontBorderType(int type){dxFont_.SetBorderType(type);}
			void SetFontBorderColor(D3DCOLOR color){dxFont_.SetBorderColor(color);}

			POINT GetPosition(){return pos_;}
			void SetPosition(int x, int y){pos_.x = x; pos_.y = y;}
			void SetPosition(POINT pos){pos_ = pos;}
			int GetMaxWidth(){return widthMax_;}
			void SetMaxWidth(int width){widthMax_ = width;}
			int GetMaxHeight(){return heightMax_;}
			void SetMaxHeight(int height){heightMax_ = height;}
			int GetSidePitch(){return sidePitch_;}
			void SetSidePitch(int pitch){sidePitch_ = pitch;}
			int GetLinePitch(){return linePitch_;}
			void SetLinePitch(int pitch){linePitch_ = pitch;}
			RECT GetMargin(){return margin_;}
			void SetMargin(RECT margin){margin_ = margin;}
			int GetHorizontalAlignment(){return alignmentHorizontal_;}
			void SetHorizontalAlignment(int value){alignmentHorizontal_ = value;}
			int GetVerticalAlignment(){return alignmentVertical_;}
			void SetVerticalAlignment(int value){alignmentVertical_ = value;}

			D3DCOLOR GetVertexColor(){return colorVertex_;}
			void SetVertexColor(D3DCOLOR color){colorVertex_ = color;}
			bool IsPermitCamera(){return bPermitCamera_;}
			void SetPermitCamera(bool bPermit){bPermitCamera_ = bPermit;}
			bool IsSyntacticAnalysis(){return bSyntacticAnalysis_;}
			void SetSyntacticAnalysis(bool bEnable){bSyntacticAnalysis_ = bEnable;}

			std::wstring& GetText(){return text_;}
			void SetText(std::wstring text){text_ = text;}

			gstd::ref_count_ptr<Shader> GetShader(){return shader_;}
			void SetShader(gstd::ref_count_ptr<Shader> shader){shader_ = shader;}
	};

	/**********************************************************
	//DxTextStepper
	**********************************************************/
	class DxTextStepper
	{
		protected:
			int posNext_;
			std::wstring text_;
			std::wstring source_;
			int framePerSec_;
			int countNextPerFrame_;
			double countFrame_;
			void _Next();
		public:
			DxTextStepper();
			virtual ~DxTextStepper();
			void Clear();

			std::wstring GetText(){return text_;}
			void Next();
			void NextSkip();
			bool HasNext();
			void SetSource(std::wstring text);
	};

}

#endif

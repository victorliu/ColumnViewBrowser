class ColumnFileBox{
	string m_path;
	BOOL m_path_is_dir;
	
	BOOL m_showing_preview;
public:
	ColumnFileBox(const string &dir);
	
	void SetColumns(which columns, and sizes)
	void SetPath(const string &dir);
	void AddSortOrder(sort_type_t sort_by, BOOL ascending = TRUE);
	void ShowPreview(BOOL show = TRUE);
	
	
	void GetColumns(columns and sizes&);
	void GetPath(string &);
	void GetSortOrder(sort order&);
	BOOL ShowingPreview() const;
	
	void GetFiles(list of files&);
	void SetSelected(const string &file);
	
};

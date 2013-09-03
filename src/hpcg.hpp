
extern std::ofstream HPCG_fout;

struct HPCG_Params_STRUCT {
  int nx, ny, nz;
};
typedef HPCG_Params_STRUCT HPCG_Params;

extern int HPCG_Init(int *argc_p, char ***argv_p, HPCG_Params *params);
extern int HPCG_Finalize(void);


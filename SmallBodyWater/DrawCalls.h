extern GraphicResources * G;

extern SceneState scene_state;

void loadMatrix_VP(SimpleMath::Matrix & v, SimpleMath::Matrix & p);
void loadMatrix_WP(SimpleMath::Matrix & w, SimpleMath::Matrix & p);
void storeMatrix(SimpleMath::Matrix & w, SimpleMath::Matrix & wv, SimpleMath::Matrix & wvp);

inline void skydome_set_world_matrix(){
	SimpleMath::Matrix v, p;
	loadMatrix_VP(v, p);

	auto invView = SimpleMath::Matrix(scene_state.mInvView).Transpose();

	auto w = SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(invView.m[3][0], invView.m[3][1], invView.m[3][2]));
	SimpleMath::Matrix  wv = w * v;
	SimpleMath::Matrix wvp = wv * p;

	storeMatrix(w, wv, wvp);
}

inline void skydome_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	G->skyDome_model->Draw(pd3dImmediateContext, effect, inputLayout, [=]
	{
		setCustomState();
	});
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void skyPlane_set_world_matrix(){
	SimpleMath::Matrix v, p;
	loadMatrix_VP(v, p);

	scene_state.viewLightPos.x += 0.00001f;
	if (scene_state.viewLightPos.x > 1)
		scene_state.viewLightPos.x = -1;

	auto invView = SimpleMath::Matrix(scene_state.mInvView).Transpose();

	SimpleMath::Matrix w = SimpleMath::Matrix::CreateScale(10, 0.5, 10) * SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(invView.m[3][0], invView.m[3][1] + 0.3, invView.m[3][2]));
	SimpleMath::Matrix  wv = w * v;
	SimpleMath::Matrix wvp = wv * p;

	storeMatrix(w, wv, wvp);
}
inline void skyPlane_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	G->skyPlane_model->Draw(pd3dImmediateContext, effect, inputLayout, [=]
	{
		setCustomState();
	});
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void ground_set_world_matrix(){
	SimpleMath::Matrix v, p;
	loadMatrix_VP(v, p);

	SimpleMath::Matrix w = SimpleMath::Matrix::CreateScale(511.0, 1.0 / 20.0, 511.0) * SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(511.0 / 2.0f, 0.0f, 511.0 / 2.0f));
	SimpleMath::Matrix  wv = w * v;
	SimpleMath::Matrix wvp = wv * p;

	scene_state.mObject1 = w.Invert().Transpose().Transpose();

	storeMatrix(w, wv, wvp);
}
inline void ground_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	G->ground_model->Draw(pd3dImmediateContext, effect, inputLayout, [=]
	{
		setCustomState();
	});
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void water_set_world_matrix(){
	SimpleMath::Matrix v, p;
	loadMatrix_VP(v, p);

	scene_state.viewLightPos.y += 0.001f;
	if (scene_state.viewLightPos.y > 1.0f)
		scene_state.viewLightPos.y = -1.0f;

	SimpleMath::Matrix w = SimpleMath::Matrix::CreateScale(110.0f * 2, 1, 110.0f * 2) * SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(240.0f, 3.75f, 250.0f));
	SimpleMath::Matrix  wv = w * v;
	SimpleMath::Matrix wvp = wv * p;

	storeMatrix(w, wv, wvp);
}
inline void water_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	G->water_model->Draw(pd3dImmediateContext, effect, inputLayout, [=]
	{
		setCustomState();
	});
}
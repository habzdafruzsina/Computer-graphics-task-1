#include "MyApp.h"
#include "GLUtils.hpp"

#include <math.h>

CMyApp::CMyApp(void)
{
	m_vaoID = 0;
	m_vboID = 0;
	m_ibID = 0;

	m_programID = 0;
}


CMyApp::~CMyApp(void)
{
}

bool CMyApp::Init()
{
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)
	glCullFace(GL_BACK); // GL_BACK: a kamerától "elfelé" nézõ lapok, GL_FRONT: a kamera felé nézõ lapok


	Vertex vert[N + 2] = { };
	r = 0.5;

	for (int i = 0; i < N; ++i) {
		float alpha = float(i) / N * 2 * float(M_PI);
		vert[i].p = glm::vec3(r * cos(alpha), 0, r * sin(alpha));
		vert[i].c = glm::normalize(vert[i].p);
		std::cout << vert[i].p[0] << " " << vert[i].p[1] << " " << vert[i].p[2] << std::endl;
	}
	vert[N].p = glm::vec3(0, 0, 0);	//középpont
	vert[N].c = glm::normalize(glm::vec3(0, -2, 0)); //hogy ne legyenek ugyanolyan színû háromszögek
	vert[N + 1].p = glm::vec3(0, 2*r, 0); //csúcspont (2 egység magasan)
	vert[N + 1].c = glm::normalize(vert[N + 1].p);
	

	GLushort indices[(N*2 +2) * 3] = { };

	//kör alap
	for (int i = 0; i < N-1; ++i) {
		indices[i * 3] = N;
		indices[i * 3 +1] = i;
		indices[i * 3 +2] = i+1;
	}
	indices[(N-1) * 3] = N;
	indices[(N-1) * 3 + 1] = N - 1;
	indices[(N-1) * 3 + 2] = 0;


	//palást
	for (int i = 0; i < N-1; ++i) {
		indices[(N*3) + i * 3] = N + 1;
		indices[(N*3) + i * 3 + 1] = i+1;
		indices[(N*3) + i * 3 + 2] = i;
	}
	indices[(N * 3) + N * 3] = N + 1;
	indices[(N * 3) + N * 3 + 1] = 0;
	indices[(N * 3) + N * 3 + 2] = N - 1;


	glGenVertexArrays(1, &m_vaoID);
	glBindVertexArray(m_vaoID);
	
	glGenBuffers(1, &m_vboID); 
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

	glBufferData( GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,				// a VB-ben található adatok közül a 0. "indexû" attribútumait állítjuk be
		3,				// komponens szam
		GL_FLOAT,		// adatok tipusa
		GL_FALSE,		// normalizalt legyen-e
		sizeof(Vertex),	// stride (0=egymas utan)
		0				// a 0. indexû attribútum hol kezdõdik a sizeof(Vertex)-nyi területen belül
	); 

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3, 
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(sizeof(glm::vec3)) );


	glGenBuffers(1, &m_ibID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	GLuint vs_ID = loadShader(GL_VERTEX_SHADER,		"myVert.vert");
	GLuint fs_ID = loadShader(GL_FRAGMENT_SHADER,	"myFrag.frag");

	m_programID = glCreateProgram();

	glAttachShader(m_programID, vs_ID);
	glAttachShader(m_programID, fs_ID);


	glBindAttribLocation( m_programID, 0, "vs_in_pos");
	glBindAttribLocation( m_programID, 1, "vs_in_col");

	glLinkProgram(m_programID);

	GLint infoLogLength = 0, result = 0;

	glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( GL_FALSE == result )
	{
		std::vector<char> ProgramErrorMessage( infoLogLength );
		glGetProgramInfoLog(m_programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
		
		char* aSzoveg = new char[ProgramErrorMessage.size()];
		memcpy( aSzoveg, &ProgramErrorMessage[0], ProgramErrorMessage.size());

		std::cout << "[app.Init()] Sáder Huba panasza: " << aSzoveg << std::endl;

		delete aSzoveg;
	}


	glDeleteShader( vs_ID );
	glDeleteShader( fs_ID );


	m_matProj = glm::perspective( 45.0f, 640/480.0f, 1.0f, 1000.0f );

	m_loc_mvp = glGetUniformLocation( m_programID, "MVP");

	return true;
}

void CMyApp::Clean()
{
	glDeleteBuffers(1, &m_vboID);
	glDeleteBuffers(1, &m_ibID);
	glDeleteVertexArrays(1, &m_vaoID);

	glDeleteProgram( m_programID );
}

void CMyApp::Update()
{
	m_matView = glm::lookAt(glm::vec3( 4,  10,  10),		// honnan nézzük a színteret
							glm::vec3( 0,  0,  0),		// a színtér melyik pontját nézzük
							glm::vec3( 0,  1,  0));		// felfelé mutató irány a világban
}


void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shader bekapcsolasa
	glUseProgram( m_programID );


	float time = SDL_GetTicks() / 1000.0f;
	float alpha = time * 2 * M_PI / 3.f;
	float beta = time * 2* M_PI / 3.f;

	float a;
	float max = 2, min = 0.5;

	if (space == true) {	//ha egyszer lenyomták a space-t, változzon a métere a kúpoknak
		a = (r*max - r*min) * sin(time * M_PI / 10) + (r*max + r*min);  //az eredeti fele és kétszerese között, 10 másodperces periódussal
	}
	else {
		a = 1;
	}

	for (int i = 0; i < 5; i++) {
		m_matWorld =
			glm::translate<float>(glm::vec3( r*8*sin(alpha)*cos(beta), r*8*cos(alpha), r*8*sin(beta)*(-sin(alpha))) ) * //kör vonallán mozog
			glm::rotate<float>(2 * M_PI / 5 * i, glm::vec3(0, 1, 0)) *  //egyenlõ távolsára legyenek az objektumok
			glm::translate<float>(glm::vec3(5*r, 0, 0))	*		// távolabb helyezzük a középponttól
			glm::scale<float>(glm::vec3(1, 1, 1*a))		//méretváltozás a z tengely irányába
			;

		drawCone(m_matWorld, N);
	}


	/*m_matWorld =
		glm::scale(glm::vec3(1, 1, 1)) *
		glm::rotate<float>(-M_PI / 2, glm::vec3(0, 1, 0));

	drawCone(m_matWorld, N);*/


	glBindVertexArray(0);

	glUseProgram( 0 );
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	switch(key.keysym.sym) {
		case SDLK_SPACE:
			space = true;
	}
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_matProj = glm::perspective(  45.0f,		// 90 fokos nyilasszog
									_w/(float)_h,	// ablakmereteknek megfelelo nezeti arany
									0.01f,			// kozeli vagosik
									100.0f);		// tavoli vagosik
}


void CMyApp::drawCone(const glm::mat4& world, const int N){

	glBindVertexArray(m_vaoID);

	glm::mat4 mvp = m_matProj * m_matView * world;

	glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, &(mvp[0][0]));

	// kirajzolás
	glDrawElements(GL_TRIANGLES,		// primitív típus
		(N *2 + 2) * 3,				// hany csucspontot hasznalunk a kirajzolashoz
		GL_UNSIGNED_SHORT,	// indexek tipusa
		0);					// indexek cime
}

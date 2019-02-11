GlslProgram:

je nach Ordnerstruktur müssen Sie die GLAD & GLM include Pfade im glsl.h File anpassen!

sollte beim Kompilieren die Meldung "Konvertierung von 'LPCWSTR' in 'LPCSTR' nicht möglich" erscheinen, stellen Sie in den Projekteigenschaften Zeichensatz von Multibyte auf Unicode um.

Verwendung:

anlegen mit
std::shared_pointer<cg2::GlslProgram> prog = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_SHADER_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_SHADER_PFAD) } });

als aktives OpenGL Programm Objekt setzen:
cg2::GlslProgram::setActiveProgram(prog);
-> muss immer aufgerufen werden, bevor uniforms gesetzt bzw. bevor damit gerendert wird;
-> gängige Uniforms wie z.B. Model/View/Projection Matrix am falschen/nicht aktiven GlslProgram zu setzen ist ein extrem häufiger Fehler,
	wenn mehr als ein GlslProgram verwendet wird (was ab der 2ten HÜ eigtl. immer der Fall ist)
	
für die Methoden zum Setzen verschiedener Uniforms bitte ins glsl.h File schauen, es gibt eigtl. alle für die HÜs Benötigten.

----------------------

Sample:

je nach Ordnerstruktur müssen Sie die GLAD & GLM include Pfade im sample.h File anpassen!

Sie legen eine neue Klasse MY_SAMPLE an, welche von cg2::Sample ableitet.
Sie müssen dabei einige Methoden implementieren, schauen Sie dafür ins sample.h file.
Weiters braucht Ihre Klasse einen public Ctor, welcher die Fensterdimensionen annimmt: MY_SAMPLE(unsigned int w, unsigned int h)


Ihre main() Funktion muss dann so aussehen:
(sorgt dafür, dass eine Instanz von MY_SAMPLE erzeugt & alle Funktionen zu richtigen Zeitpunkt aufgerufen werden)

int main()
{
	if (!glfwInit())
		return -1;

	std::shared_ptr<cg2::SampleWindow> window = cg2::SampleWindow::create<MY_SAMPLE>("Cube Demo", 768, 768, 3, 3, 8);
	if (window == nullptr)
	{
		glfwTerminate();
		return -1;
	}

	while (1)
	{
		if (window->shouldClose())
		{
			window.reset();
			break;
		}

		window->onNewFrame();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

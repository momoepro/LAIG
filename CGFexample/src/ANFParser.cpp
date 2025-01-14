#include "ANFParser.h"
#include "Perspective.h"
#include <string.h>
#include "Orthocamera.h"

#include <math.h>

#define pi acos(-1.0)

ANFParser::ANFParser(char *filename)
{

	// Read ANF from file

	doc=new TiXmlDocument( filename );
	bool loadOkay = doc->LoadFile();

	if ( !loadOkay )
	{
		printf( "Could not load file '%s'. Error='%s'. Exiting.\n", filename, doc->ErrorDesc() );
		exit( 1 );
	}
	string fname(filename);
	this->filename = fname;
	TiXmlElement* anfElement= doc->FirstChildElement( "anf" );

	if (anfElement == NULL)
	{
		printf("Main anf block element not found! Exiting!\n");
		exit(1);
	}

	globalsElement = anfElement->FirstChildElement( "globals" );
	camerasElement = anfElement->FirstChildElement( "cameras" );
	lightsElement =  anfElement->FirstChildElement( "lights" );
	texturesElement =  anfElement->FirstChildElement( "textures" );
	appearancesElement =  anfElement->FirstChildElement( "appearances" );
	graphElement =  anfElement->FirstChildElement( "graph" );
	animationElement = anfElement->FirstChildElement("animations");
	lightid = 1;

	if (globalsElement == NULL)
		printf("Globals block not found!\n");
	else
	{
		printf("Processing globals:\n");
		// drawing mode: example of a node with individual attributes
		TiXmlElement* drawingElement=globalsElement->FirstChildElement("drawing");
		if (drawingElement)
		{
			char *background=NULL;

			drawing_mode=(char *) drawingElement->Attribute("mode");
			if(strcmp(drawing_mode,"fill") != 0 && strcmp(drawing_mode,"line") != 0  && strcmp(drawing_mode,"point") !=0){
				printf("Invalid mode value. Fill value will be taken for mode.\n");
				drawing_mode = "fill";
			}
			drawing_shading=(char *) drawingElement->Attribute("shading");
			if(strcmp(drawing_shading,"flat") != 0 && strcmp(drawing_shading,"gouraud") != 0){
				printf("Invalid shading value. Flat will be taken for shading.\n");
				drawing_shading = "flat";
			}
			printf("mode: %s, shading: %s\n",drawing_mode,drawing_shading);
			background=(char *) drawingElement->Attribute("background");
			if(background && sscanf(background,"%f %f %f %f",&global_background[0], &global_background[1],&global_background[2],&global_background[3])==4)
			{
				printf("background values (XYWZ): %f %f %f %f\n", global_background[0], global_background[1],global_background[2],global_background[3]);
			}
			else{
				printf("Invalid background values. 0 0 0 1 will be taken for background\n");
				global_background[0]=0;
				global_background[1]=0;
				global_background[2]=0;
				global_background[3]=1;
			}
		}
		else
			printf("drawing not found\n");

		// translate: example of a node with an attribute comprising several float values
		// It shows an example of extracting an attribute's value, and then further parsing that value
		// to extract individual values
		TiXmlElement* cullingElement=globalsElement->FirstChildElement("culling");
		if (cullingElement)
		{

			culling_face=(char *) cullingElement->Attribute("face");
			if(strcmp(culling_face,"none") != 0 && strcmp(culling_face,"back") != 0 && strcmp(culling_face,"front") != 0 && strcmp(culling_face,"both") != 0){
				printf("Invalid face values. back will be taken for face\n");
				culling_face = "back";
			}
			culling_order=(char *) cullingElement->Attribute("order");
			if(strcmp(culling_order,"ccw") != 0 && strcmp(culling_order,"cw") != 0){
				printf("Invalid order values. counterclockwise will be taken for order\n");
				culling_order = "ccw";
			}
			printf("face: %s, order: %s\n",culling_face,culling_order);
		}
		else
			printf("culling not found\n");

		TiXmlElement* lightingElement=globalsElement->FirstChildElement("lighting");
		if (lightingElement)
		{
			char *ambient=NULL;
			if (lightingElement->QueryBoolAttribute("doublesided",&doublesided)==TIXML_SUCCESS &&
				lightingElement->QueryBoolAttribute("local",&local)==TIXML_SUCCESS &&
				lightingElement->QueryBoolAttribute("enabled",&enabled)==TIXML_SUCCESS
				)
				printf("  lightning attributes: %d %d %d\n", doublesided, local,enabled);
			else
				printf("Error parsing lightning\n");
			ambient=(char *) lightingElement->Attribute("ambient");
			if(ambient && sscanf(ambient,"%f %f %f %f",&global_ambient[0], &global_ambient[1],&global_ambient[2], &global_ambient[3])==4)
			{
				printf("ambient values (XYWZ): %f %f %f %f\n", global_ambient[0], global_ambient[1],global_ambient[2],global_ambient[3]);
			}
			else{
				printf("Invalid ambient values. 0 0 0 1 will be taken for ambient\n");
				global_ambient[0]=0;
				global_ambient[1]=0;
				global_ambient[2]=0;
				global_ambient[3]=1;
			}
		}
		else
			printf("lightning not found\n");
	}
	if (camerasElement == NULL)
		printf("Globals block not found!\n");
	else
	{
		initial_camera=(char *) camerasElement->Attribute("initial");
		if(!initial_camera){
			printf("Invalid initial value. the first perspective camera will be used.\n");
		}
		printf("camera initial: %s\n",initial_camera);
		TiXmlElement* perspectiveElement=camerasElement->FirstChildElement("perspective");
		while (perspectiveElement)
		{
			char *id=NULL, *pos =NULL, *target = NULL;
			float near, far, angle, positions[3],targets[3];

			id=(char *) perspectiveElement->Attribute("id");
			if(!id){
				printf("Invalid id value. inv value will be taken for id.\n");
				id = "inv";
			}
			printf("perspective id: %s\n",id);
			if (perspectiveElement->QueryFloatAttribute("near",&near)==TIXML_SUCCESS &&
				perspectiveElement->QueryFloatAttribute("far",&far)==TIXML_SUCCESS &&
				perspectiveElement->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS
				)
				printf("near: %f, far: %f, angle: %f\n", near, far, angle);
			else
				printf("Error parsing near,far,angle\n");
			pos=(char *) perspectiveElement->Attribute("pos");
			if(pos && sscanf(pos,"%f %f %f",&positions[0], &positions[1],&positions[2])==3)
			{
				printf("pos values (XYZ): %f %f %f\n", positions[0], positions[1], positions[2]);
			}
			else{
				printf("Invalid pos values. 0 0  1 will be taken for pos\n");
				positions[0]=0;
				positions[1]=0;
				positions[2]=1;
			}
			target=(char *) perspectiveElement->Attribute("target");
			if(target && sscanf(pos,"%f %f %f",&targets[0], &targets[1],&targets[2])==3)
			{
				printf("target values (XYZ): %f %f %f\n", targets[0], targets[1], targets[2]);
			}
			else{
				printf("Invalid target values. 0 0  1 will be taken for target\n");
				targets[0]=0;
				targets[1]=0;
				targets[2]=1;
			}
			Perspective *p_camera;
			p_camera = new Perspective(id,near,far,angle,positions,targets);
			cameras.push_back(*p_camera);
			if(!initial_camera) {
				initial_camera = p_camera->getId();
			}
			perspectiveElement = perspectiveElement->NextSiblingElement("perspective");
		}
		TiXmlElement* orthoElement=camerasElement->FirstChildElement("ortho");
		while (orthoElement)
		{
			char *id=NULL, *direction;
			float near, far, left, right, top, bottom;

			id=(char *) orthoElement->Attribute("id");
			if(!id){
				printf("Invalid id value. inv value will be taken for id.\n");
				id = "inv";
			}
			printf("orthos id: %s\n",id);
			direction=(char *) orthoElement->Attribute("direction");
			if(!direction){
				printf("Invalid id value. inv value will be taken for id.\n");
				direction = "y";
			}
			printf("direction: %s\n",direction);
			if (orthoElement->QueryFloatAttribute("near",&near)==TIXML_SUCCESS &&
				orthoElement->QueryFloatAttribute("far",&far)==TIXML_SUCCESS &&
				orthoElement->QueryFloatAttribute("left",&left)==TIXML_SUCCESS  &&
				orthoElement->QueryFloatAttribute("right",&right)==TIXML_SUCCESS&&
				orthoElement->QueryFloatAttribute("top",&top)==TIXML_SUCCESS &&
				orthoElement->QueryFloatAttribute("bottom",&bottom)==TIXML_SUCCESS
				)
				printf("near: %f, far: %f, left: %f, right: %f, top: %f, bottom: %f\n", near, far, left, right, top , bottom);
			else
				printf("Error parsing near,far,left, right, top, bottom\n");
			Orthocamera *o_camera;
			o_camera = new Orthocamera(*direction,left,right, bottom, top , near, far);
			cameras.push_back(*o_camera);
			if(!initial_camera)
				initial_camera=o_camera->getId();
			orthoElement = orthoElement->NextSiblingElement("ortho");

		}
		//else
		//	printf("cameras not found\n");
	}
	if (lightsElement == NULL)
		printf("Lights block not found!\n");
	else
	{
		printf("Processing lights:\n");
		// drawing mode: example of a node with individual attributes
		TiXmlElement* lightElement = lightsElement->FirstChildElement("light");
		lightid = GL_LIGHT0;
		while (lightElement)
		{
			char *id = NULL, *type = NULL, *pos, *target = NULL;
			float lpos[3], ltarget[3], angle, exponent;
			bool enabled, marker;

			id = (char *)lightElement->Attribute("id");
			if (!id){
				printf("Invalid id value. inv value will be taken for id.\n");
				id = "inv";
			}
			printf("light id: %s\n", id);

			type = (char *)lightElement->Attribute("type");
			if (!type){
				printf("Invalid type value. inv value will be taken for type.\n");
				type = "inv";
			}
			printf("light type: %s\n", type);

			if (lightElement->QueryBoolAttribute("enabled", &enabled) == TIXML_SUCCESS &&
				lightElement->QueryBoolAttribute("marker", &marker) == TIXML_SUCCESS)
				printf("  light attributes: %d %d\n", enabled, marker);
			else
				printf("Error parsing lightning\n");

			pos = (char *)lightElement->Attribute("pos");
			if (pos && sscanf(pos, "%f %f %f", &lpos[0], &lpos[1], &lpos[2]) == 3)
			{
				printf("pos values (XYZ): %f %f %f\n", lpos[0], lpos[1], lpos[2]);
			}
			else{
				printf("Invalid pos values. 0 0  1 will be taken for pos\n");
				lpos[0] = 0;
				lpos[1] = 0;
				lpos[2] = 1;
			}
			target = (char *)lightElement->Attribute("target");
			if (target && sscanf(target, "%f %f %f", &ltarget[0], &ltarget[1], &ltarget[2]) == 3){
				printf("target values (XYZ): %f %f %f\n", ltarget[0], ltarget[1], ltarget[2]);
			}
			if (lightElement->QueryFloatAttribute("angle", &angle) == TIXML_SUCCESS &&
				lightElement->QueryFloatAttribute("exponent", &exponent) == TIXML_SUCCESS)
				printf("  light attributes: %f %f\n", angle, exponent);
			else
				printf("Error parsing lightning\n");
			TiXmlElement* componentElement = lightElement->FirstChildElement("component");

			char *ambient, *ambientvalue, *diffuse, *diffusevalue, *specular, *specularvalue;
			float amb[4], dif[4], spec[4];

			ambient = (char *)componentElement->Attribute("type");
			printf("component type: %s\n", ambient);
			ambientvalue = (char *)componentElement->Attribute("value");
			if (ambientvalue && sscanf(ambientvalue, "%f %f %f %f", &amb[0], &amb[1], &amb[2], &amb[3]) == 4)
			{
				printf("ambient values (XYWZ): %f %f %f %f\n", amb[0], amb[1], amb[2], amb[3]);
			}
			else{
				printf("Invalid ambient values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
				amb[0] = 0;
				amb[1] = 0;
				amb[2] = 1;
				amb[3] = 1;
			}
			componentElement = componentElement->NextSiblingElement("component");
			diffuse = (char *)componentElement->Attribute("type");
			printf("component type: %s\n", diffuse);
			diffusevalue = (char *)componentElement->Attribute("value");
			if (diffusevalue && sscanf(diffusevalue, "%f %f %f %f", &dif[0], &dif[1], &dif[2], &dif[3]) == 4)
			{
				printf("diffuse values (XYWZ): %f %f %f %f\n", dif[0], dif[1], dif[2], dif[3]);
			}
			else{
				printf("Invalid diffuse values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
				dif[0] = 0;
				dif[1] = 0;
				dif[2] = 1;
				dif[3] = 1;
			}
			componentElement = componentElement->NextSiblingElement("component");
			specular = (char *)componentElement->Attribute("type");
			printf("component type: %s\n", specular);
			specularvalue = (char *)componentElement->Attribute("value");
			if (specularvalue && sscanf(specularvalue, "%f %f %f %f", &spec[0], &spec[1], &spec[2], &spec[3]) == 4)
			{
				printf("specular values (XYWZ): %f %f %f %f\n", spec[0], spec[1], spec[2], spec[3]);
			}
			else{
				printf("specular ambient values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
				spec[0] = 0;
				spec[1] = 0;
				spec[2] = 1;
				spec[3] = 1;
			}
			myLight *newLight;
			if (!target){

				newLight = new myLight(lightid, lpos, id, enabled, marker);
				newLight->setAmbient(amb);
				newLight->setDiffuse(dif);
				newLight->setSpecular(spec);
			}
			else{
				newLight = new myLight(lightid, lpos, ltarget, id, enabled, marker);
				newLight->setAmbient(amb);
				newLight->setDiffuse(dif);
				newLight->setSpecular(spec);
				newLight->setAngle(angle);
				newLight->setExponent(exponent);
			}
			lights.push_back(*newLight);
			lightid++;
			lightElement = lightElement->NextSiblingElement("light");

		}
		if (texturesElement == NULL)
			printf("Textures block not found!\n");
		else
		{
			printf("Processing textures:\n");
			TiXmlElement* textureElement = texturesElement->FirstChildElement("texture");
			while (textureElement){
				char *id = NULL, *file = NULL;
				float texlength_s, texlength_t;

				id = (char *)textureElement->Attribute("id");
				if (!id){
					printf("Invalid id value. inv value will be taken for id.\n");
					id = "inv";
				}
				printf("texture id: %s\n", id);

				file = (char *)textureElement->Attribute("file");
				if (!id){
					printf("Invalid file value. inv value will be taken for file.\n");
					file = "inv";
				}
				printf("texture file: %s\n", file);

				if (textureElement->QueryFloatAttribute("texlength_s", &texlength_s) == TIXML_SUCCESS &&
					textureElement->QueryFloatAttribute("texlength_t", &texlength_t) == TIXML_SUCCESS)
					printf("Texture attributes: %f %f\n", texlength_s, texlength_t);

				myTexture *newTexture = new myTexture(id, file, texlength_s, texlength_t);
				textures.push_back(*newTexture);
				textureElement = textureElement->NextSiblingElement("texture");
			}
		}
		if (appearancesElement == NULL)
			printf("Appearances block not found!\n");
		else
		{
			printf("Processing Appearances:\n");
			// drawing mode: example of a node with individual attributes
			TiXmlElement* appearanceElement = appearancesElement->FirstChildElement("appearance");
			while (appearanceElement)
			{
				char *id = NULL, *textureref = NULL;
				float shininess;

				id = (char *)appearanceElement->Attribute("id");
				if (!id){
					printf("Invalid id value. inv value will be taken for id.\n");
					id = "inv";
				}
				printf("appearance id: %s\n", id);

				if (appearanceElement->QueryFloatAttribute("shininess", &shininess) == TIXML_SUCCESS)
					printf("shininess attribute: %f\n", shininess);

				textureref = (char *)appearanceElement->Attribute("textureref");
				TiXmlElement* componentElement = appearanceElement->FirstChildElement("component");

				char *ambient, *ambientvalue, *diffuse, *diffusevalue, *specular, *specularvalue;
				float amb[4], dif[4], spe[4];

				ambient = (char *)componentElement->Attribute("type");
				printf("component type: %s\n", ambient);
				ambientvalue = (char *)componentElement->Attribute("value");
				if (ambientvalue && sscanf(ambientvalue, "%f %f %f %f", &amb[0], &amb[1], &amb[2], &amb[3]) == 4)
				{
					printf("ambient values (XYWZ): %f %f %f %f\n", amb[0], amb[1], amb[2], amb[3]);
				}
				else{
					printf("Invalid ambient values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
					amb[0] = 0;
					amb[1] = 0;
					amb[2] = 1;
					amb[3] = 1;
				}
				componentElement = componentElement->NextSiblingElement("component");
				diffuse = (char *)componentElement->Attribute("type");
				printf("component type: %s\n", diffuse);
				diffusevalue = (char *)componentElement->Attribute("value");
				if (diffusevalue && sscanf(diffusevalue, "%f %f %f %f", &dif[0], &dif[1], &dif[2], &dif[3]) == 4)
				{
					printf("diffuse values (XYWZ): %f %f %f %f\n", dif[0], dif[1], dif[2], dif[3]);
				}
				else{
					printf("Invalid diffuse values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
					dif[0] = 0;
					dif[1] = 0;
					dif[2] = 1;
					dif[3] = 1;
				}
				componentElement = componentElement->NextSiblingElement("component");
				specular = (char *)componentElement->Attribute("type");
				printf("component type: %s\n", specular);
				specularvalue = (char *)componentElement->Attribute("value");
				if (specularvalue && sscanf(specularvalue, "%f %f %f %f", &spe[0], &spe[1], &spe[2], &spe[3]) == 4)
				{
					printf("specular values (XYWZ): %f %f %f %f\n", spe[0], spe[1], spe[2], spe[3]);
				}
				else{
					printf("specular ambient values. 0 0 1 1 will be taken for a1,a2,a3,a4\n");
					spe[0] = 0;
					spe[1] = 0;
					spe[2] = 1;
					spe[3] = 1;
				}

				myAppearance *newAppearance;
				newAppearance = new myAppearance(id, shininess);
				newAppearance->setAmbient(amb);
				newAppearance->setDiffuse(dif);
				newAppearance->setSpecular(spe);
				if (textureref == NULL){
					newAppearance->setTexRef("");
				}
				else
					newAppearance->setTexRef(textureref);

				appearances.push_back(*newAppearance);
				appearanceElement = appearanceElement->NextSiblingElement("appearance");
			}
		}
		if (animationElement == NULL)
			printf("Animation block not found!\n");
		else {
			TiXmlElement* aElement = animationElement->FirstChildElement("animation");
			while (aElement)
			{
				char *id = NULL, *type, *center;
				float span, controlpointx, controlpointy, controlpointz;
				float centerx, centery, centerz, radius, startang, rotang;
				id = (char *)aElement->Attribute("id");
				printf("animation id: %s\n", id);
				if (aElement->QueryFloatAttribute("span", &span) == TIXML_SUCCESS){
				}
				else{
					printf("Invalid span. 10 will be taken for angle value.");
					span = 10.0;
				}
				printf("SPAN: %f\n", span);
				type = (char *)aElement->Attribute("type");
				printf("animation type = %s\n", type);
				if (strcmp(type, "linear") == 0) {
					std::vector<Point> vecP;
					TiXmlElement *controlpoint = aElement->FirstChildElement("controlpoint");
					while (controlpoint) {
						if (controlpoint->QueryFloatAttribute("xx", &controlpointx) == TIXML_SUCCESS &&
							controlpoint->QueryFloatAttribute("yy", &controlpointy) == TIXML_SUCCESS &&
							controlpoint->QueryFloatAttribute("zz", &controlpointz) == TIXML_SUCCESS)
							printf("CONTROL Point X: %f, Y: %f, Z: %f \n", controlpointx, controlpointy, controlpointz);

						Point p(controlpointx, controlpointy, controlpointz);
						vecP.push_back(p);

						controlpoint = controlpoint->NextSiblingElement("controlpoint");
					}
					LinearAnimation *la = new LinearAnimation(id, span, vecP);
					Animation *a = new Animation(*la);
					string nome(id);
					animations[nome] = a;
				}
				else if (strcmp(type, "circular") == 0) {
					center = (char *)aElement->Attribute("center");
					if (center && sscanf(center, "%f %f %f", &centerx, &centery, &centerz) == 3)
					{
						printf("Center (XYZ): %f %f %f %f\n", centerx, centery, centerz);
					}
					else{
						printf("Center values. 0 0 0 will be taken for centerx,centery,centerz\n");
						centerx = 0;
						centery = 0;
						centerz = 0;
					}
					if (aElement->QueryFloatAttribute("radius", &radius) == TIXML_SUCCESS &&
						aElement->QueryFloatAttribute("startang", &startang) == TIXML_SUCCESS &&
						aElement->QueryFloatAttribute("rotang", &rotang) == TIXML_SUCCESS)
						printf("Radius: %f, Startang: %f, Rotang: %f \n", radius, startang, rotang);

					CircularAnimation *ca = new CircularAnimation(id, span, Point(centerx, centery, centerz), radius, startang, rotang);
					Animation *a = new Animation(*ca);
					string nome(id);
					animations[nome] = a;
				}
				aElement = aElement->NextSiblingElement("animation");
			}
		}
		if (graphElement == NULL)
			printf("Graph block not found!\n");
		else
		{
			char *rootid=NULL;
			rootid=(char *) graphElement->Attribute("rootid");
			if(!rootid){
				printf("Invalid rootid value. inv value will be taken for rootid.\n");
				rootid = "inv";
			}
			printf("root id: %s\n",rootid);
			graph = new SceneGraph(rootid);
			TiXmlElement *node=graphElement->FirstChildElement("node");

			while (node)
			{
				char *id=NULL;
				id=(char *) node->Attribute("id");
				if(!id){
					printf("Invalid node id value. inv value will be taken for node id.\n");
					id = "inv";
				}
				printf("node id: %s\n",id);
				bool displaylist;
				if(node->QueryBoolAttribute("displaylist",&displaylist)==TIXML_SUCCESS){
					printf("Displaylist : %d\n",displaylist);
				}
                else
                    displaylist= false;

				Node *newNode = new Node(id);

				TiXmlElement *transforms=node->FirstChildElement("transforms");
				if(transforms){
					TiXmlElement *transformElement=transforms->FirstChildElement("transform");
					while (transformElement){
						char *type=NULL;
						//float transform[16];
						vector<float> transform;
						type=(char *) transformElement->Attribute("type");
						if(!type){
							printf("Invalid rootid value. inv value will be taken for rootid.\n");
							type = "inv";
						}
						if(strcmp(type,"translate") == 0) {
							char *to;
							float x,y,z;
							to=(char *) transformElement->Attribute("to");
							if(to && sscanf(to,"%f %f %f",&x, &y,&z)==3)
							{
								printf("translate values (XYW): %f %f %f\n", x, y, z);
							}
							else{
								printf("wrong translate values. 0 0 0  will be taken for x,y,z\n");
								x = 0;
								y = 0;
								z = 0;
							}
							transform.push_back(1);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(x);
							transform.push_back(0);
							transform.push_back(1);
							transform.push_back(0);
							transform.push_back(y);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(1);
							transform.push_back(z);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(1);

						}
						else if(strcmp(type,"rotate") == 0) {
							char *axis;
							float angle;

							axis=(char*) transformElement->Attribute("axis");
							if(!axis && (axis != "x" && axis !="y" && axis !="z")){
								printf("Invalid axis value. x value will be taken for axis.\n");
								axis = "x";
							}
							printf("axis: %s\n",axis);
							if (transformElement->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS ){
							}
							else{
								printf("Invalid rotation angle. 0 will be taken for angle value.");
								angle = 0;
							}
							angle = pi * angle /180;
																											
							if(strcmp(axis,"z") == 0){
								transform.push_back(cos(angle));
								transform.push_back(sin(angle));
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(-sin(angle));
								transform.push_back(cos(angle));
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(1);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(1);
							}
							else if(strcmp(axis,"x") == 0){
								transform.push_back(1);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(cos(angle));
								transform.push_back(sin(angle));
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(-sin(angle));
								transform.push_back(cos(angle));
								transform.push_back(0);
								transform.push_back(0);					
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(1);
							}
							else if(strcmp(axis,"y") == 0){
								transform.push_back(cos(angle));
								transform.push_back(0);
								transform.push_back(-sin(angle));
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(1);
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(sin(angle));
								transform.push_back(0);
								transform.push_back(cos(angle));
								transform.push_back(0);
								transform.push_back(0);					
								transform.push_back(0);
								transform.push_back(0);
								transform.push_back(1);
							}
							printf("Rotate angle: %f\n", angle);

						}
						else if(strcmp(type,"scale") == 0) {
							char *factor;
							float x,y,z;
							factor=(char *) transformElement->Attribute("factor");
							if(factor && sscanf(factor,"%f %f %f",&x, &y,&z)==3)
							{
								printf("scale values (XYW): %f %f %f\n", x, y, z);
							}
							else{
								printf("wrong scale values. 1 1 1  will be taken for x,y,z\n");
								x = 1;
								y = 1;
								z = 1;
							}
							transform.push_back(x);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(y);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(z);
							transform.push_back(0);
							transform.push_back(0);					
							transform.push_back(0);
							transform.push_back(0);
							transform.push_back(1); 

						}
						newNode->addTransform(transform);
						transformElement=transformElement->NextSiblingElement();
					}
				}
				TiXmlElement *appearanceref=node->FirstChildElement("appearanceref");
				if(appearanceref == NULL) {
					newNode->setAppearance("");
				}
				else {
					char *appearancerefid = NULL;
					appearancerefid=(char *) appearanceref->Attribute("id");
					if(!appearancerefid){
						printf("Invalid appearanceref id value. inv value will be taken for node id.\n");
						appearancerefid = "inv";
					}
					printf("appearanceref id: %s\n",appearancerefid);
					newNode->setAppearance(appearancerefid);
				}
				TiXmlElement *animationref=node->FirstChildElement("animationref");
				while(animationref != NULL){
					char *animationID = NULL;
					animationID = (char *) animationref->Attribute("id");
					if(!animationID){
						printf("Invalid animation id value. inv value will be taken for node id.\n");
						animationID = "inv";
					}
					printf("animationID id: %s\n",animationID);
					string n(animationID);
					newNode->addAnimation(animations[n]);
					animationref = animationref->NextSiblingElement("animationref");
				}
				TiXmlElement *primitives=node->FirstChildElement("primitives");
				if(primitives != NULL) {
					TiXmlElement *rectangle=primitives->FirstChildElement("rectangle");
					while(rectangle){
						char *xi,*xf;
						float xi1,xi2,xf1,xf2;
						xi=(char *) rectangle->Attribute("xy1");
						if(xi && sscanf(xi,"%f %f ",&xi1, &xi2)==2)
						{
							printf("rectangle values (xi1,xi2): %f %f\n", xi1, xi2);
						}
						else{
							printf("wrong translate values. 0 0  will be taken for xi1,xi2\n");
							xi1 = 0;
							xi2 = 0;
						}
						xf=(char *) rectangle->Attribute("xy2");
						if(xf && sscanf(xf,"%f %f ",&xf1, &xf2)==2)
						{
							printf("rectangle values (xf2,xf2): %f %f\n", xf1, xf2);
						}
						else{
							printf("wrong translate values. 0 0  will be taken for xf1,xf2\n");
							xf1 = 0;
							xf2 = 0;
						}

						Rectangle *newRectangle = new Rectangle(xi1, xi2, xf1, xf2);
						Primitive *newPrimitive = new Primitive(*newRectangle);
						newNode->addPrimitive(*newPrimitive);

						rectangle = rectangle->NextSiblingElement("rectangle");
					}
					TiXmlElement *triangle=primitives->FirstChildElement("triangle");
					while(triangle){
						char *p1,*p2,*p3;
						float p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z;
						p1=(char *) triangle->Attribute("xyz1");
						if(p1 && sscanf(p1,"%f %f %f",&p1x, &p1y,&p1z)==3)
						{
							printf("triangle values (xyz1): %f %f %f\n", p1x, p1y, p1z);
						}
						else{
							printf("Wrong triangle xyz1 values. 0 0 0  will be taken for x,y,z\n");
							p1x = 0;
							p1y = 0;
							p1z = 0;
						}
						p2=(char *) triangle->Attribute("xyz2");
						if(p2 && sscanf(p2,"%f %f %f",&p2x, &p2y,&p2z)==3)
						{
							printf("triangle values (xyz2): %f %f %f\n", p2x, p2y, p2z);
						}
						else{
							printf("Wrong triangle xyz1 values. 1 1 1  will be taken for x,y,z\n");
							p2x = 1;
							p2y = 1;
							p2z = 1;
						}
						p3=(char *) triangle->Attribute("xyz3");
						if(p3 && sscanf(p3,"%f %f %f",&p3x, &p3y,&p3z)==3)
						{
							printf("triangle values (xyz3): %f %f %f\n", p3x, p3y, p3z);
						}
						else{
							printf("Wrong triangle xyz3 values. 2 2 2  will be taken for x,y,z\n");
							p3x = 2;
							p3y = 2;
							p3z = 2;
						}

						Triangle *newTriangle = new Triangle(p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z);
						Primitive *newPrimitive = new Primitive(*newTriangle);
						newNode->addPrimitive(*newPrimitive);

						triangle = triangle->NextSiblingElement("triangle");
					}
					TiXmlElement *cylinder=primitives->FirstChildElement("cylinder");
					while(cylinder){
						float base,top,height;
						int slices, stacks;
						if (cylinder->QueryFloatAttribute("base",&base)==TIXML_SUCCESS &&
							cylinder->QueryFloatAttribute("top",&top)==TIXML_SUCCESS &&
							cylinder->QueryFloatAttribute("height",&height)==TIXML_SUCCESS)
							printf("base: %f, top : %f, height: %f\n", base,top,height);

						if (cylinder->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS &&
							cylinder->QueryIntAttribute("stacks",&stacks)==TIXML_SUCCESS)
							printf("slices: %d, stacks : %d \n", slices,stacks);

						Cylinder *newCylinder = new Cylinder(base, top, height, slices, stacks);
						Primitive *newPrimitive = new Primitive(*newCylinder);
						newNode->addPrimitive(*newPrimitive);

						cylinder = cylinder->NextSiblingElement("cylinder");
					}
					TiXmlElement *sphere=primitives->FirstChildElement("sphere");
					while(sphere){
						float radius;
						int slices, stacks;
						if (sphere->QueryFloatAttribute("radius",&radius)==TIXML_SUCCESS &&
							sphere->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS &&
							sphere->QueryIntAttribute("stacks",&stacks)==TIXML_SUCCESS)
							printf("radius: %f, slices: %d, stacks : %d \n",radius, slices,stacks);

						Sphere *newSphere = new Sphere(radius, slices, stacks);
						Primitive *newPrimitive = new Primitive(*newSphere);
						newNode->addPrimitive(*newPrimitive);

						sphere = sphere->NextSiblingElement("sphere");
					}

					TiXmlElement *torus=primitives->FirstChildElement("torus");
					while(torus){
						float inner, outer;
						int slices, loops;
						if (torus->QueryFloatAttribute("inner",&inner)==TIXML_SUCCESS &&
							torus->QueryFloatAttribute("outer",&outer)==TIXML_SUCCESS &&
							torus->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS &&
							torus->QueryIntAttribute("loops",&loops)==TIXML_SUCCESS)
							printf("inner: %f, outer: %f, slices: %d, loops : %d \n",inner, outer, slices,loops);

						Torus *newTorus = new Torus(inner, outer, slices, loops);
						printf("inner: %f, outer: %f, slices: %d, loops : %d \n",inner, outer, slices,loops);
						printf("inner: %f, outer: %f, slices: %d, loops : %d \n",inner, outer, slices,loops);
						Primitive *newPrimitive = new Primitive(*newTorus);
						newNode->addPrimitive(*newPrimitive);

						torus = torus->NextSiblingElement("torus");
					}
                    TiXmlElement *plane=primitives->FirstChildElement("plane");
                    while(plane){
                        int parts;
                        if (plane->QueryIntAttribute("parts",&parts)==TIXML_SUCCESS);
                        else
                            parts= 10;
						    printf("Plane parts: %d \n",parts);
                        Plane *newPlane = new Plane(parts);
                        Primitive *newPrimitive = new Primitive(*newPlane);
                        newNode->addPrimitive(*newPrimitive);
                        plane = plane->NextSiblingElement("plane");
                    }
                    TiXmlElement *patch=primitives->FirstChildElement("patch");
                    while(patch){
                        int order, partsV,partsU;
                        char *compute;
                        float controlpointx,controlpointy,controlpointz;
                        if (patch->QueryIntAttribute("order",&order)==TIXML_SUCCESS && patch->QueryIntAttribute("partsU",&partsU)==TIXML_SUCCESS && patch->QueryIntAttribute("partsV",&partsV)==TIXML_SUCCESS)
                            printf("Patch order: %d partsU: %d partsV: %d ",order,partsU,partsV);
                        
                        compute=(char *) patch->Attribute("compute");
                        if(!compute){
                            printf("Invalid compute value. fill value will be taken for node id.\n");
                            id = "fill";
                        }
						std::vector<Point> vecP;
                        TiXmlElement *controlpoint=patch->FirstChildElement("controlpoint");
                        while(controlpoint) {
                            if (controlpoint->QueryFloatAttribute("x",&controlpointx)==TIXML_SUCCESS &&
                                controlpoint->QueryFloatAttribute("y",&controlpointy)==TIXML_SUCCESS &&
                                controlpoint->QueryFloatAttribute("z",&controlpointz)==TIXML_SUCCESS)
                                printf("CONTROL Point X: %f, Y: %f, Z: %f \n",controlpointx,controlpointy,controlpointz);
                            
							Point p(controlpointx, controlpointy, controlpointz);
							vecP.push_back(p);
                            controlpoint = controlpoint->NextSiblingElement("controlpoint");
                        }
                        
                        Patch *newPatch = new Patch(order, partsU, partsV, compute,vecP);
                        
                        Primitive *newPrimitive = new Primitive(*newPatch);
                        newNode->addPrimitive(*newPrimitive);
                        patch = patch->NextSiblingElement("patch");
                    }
					TiXmlElement *vehicle=primitives->FirstChildElement("vehicle");
                    while(vehicle){
                        Vehicle *newVehicle = new Vehicle();
                        Primitive *newPrimitive = new Primitive(*newVehicle);
                        newNode->addPrimitive(*newPrimitive);
                        vehicle = vehicle->NextSiblingElement("vehicle");
                    }
					TiXmlElement *flag=primitives->FirstChildElement("flag");
                    while(flag){
                        char * texture;
						texture=(char *) flag->Attribute("texture");
						printf("Flag texture: %s. \n",texture);
                        Flag *newFlag = new Flag(texture);
                        Primitive *newPrimitive = new Primitive(*newFlag);
                        newNode->addPrimitive(*newPrimitive);
                        flag = flag->NextSiblingElement("flag");
                    }
				}

				TiXmlElement *descendants=node->FirstChildElement("descendants");
				if(descendants != NULL) {
					TiXmlElement *noderef=descendants->FirstChildElement("noderef");
					while(noderef){
						char *noderefid=NULL;
						noderefid=(char *) noderef->Attribute("id");
						if(!noderefid){
							printf("Invalid node id value. inv value will be taken for node id.\n");
							noderefid = "inv";
						}
						printf("node id: %s\n",noderefid);

						newNode->addDescendant(noderefid);
						noderef = noderef->NextSiblingElement("noderef");
					}
				}
				newNode->calcTransform();
				graph->addNode(newNode);
				node=node->NextSiblingElement();
			}
		}
	}
}

char *ANFParser::getDrawingMode(){
	return drawing_mode;
}
char *ANFParser::getDrawingShading(){
	return drawing_shading;
}
char *ANFParser::getCullingFace(){
	return culling_face;
}
char *ANFParser::getCullingOrder(){
	return culling_order;
}
char *ANFParser::getInitialCamera(){
	return initial_camera;
}
float *ANFParser::getGlobalBackgroung(){
	return global_background;
}
float *ANFParser::getGlobalAmbient(){
	return global_ambient;
}
bool ANFParser::getDoublesided(){
	return doublesided;
}
bool ANFParser::getLocal(){
	return local;
}
bool ANFParser::getEnabled(){
	return enabled;
}
vector<myLight> ANFParser::getLights(){
	return lights;
}
vector<CGFcamera> ANFParser::getCameras(){
	return cameras;
}
vector<myTexture> ANFParser::getTextures(){
	return textures;
}
vector<myAppearance> ANFParser::getAppearances(){
	return appearances;
}
SceneGraph* ANFParser::getGraph(){
	return graph;
}

std::string ANFParser::getFileName(){
	return filename;
}

ANFParser::~ANFParser()
{
	delete(doc);
}
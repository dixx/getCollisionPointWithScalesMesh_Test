/**
 * @file    getCollisionPointWithScalesMesh_Test.cpp
 * @date    04.06.2012
 * @author  dixx
 * @brief   Test to locate gaps (points where getCollisionPoint() returns no
 *          point) within a scaled mesh
 *
 *  Additional information:
 *      Irrlicht Engine version 1.7.2
 *      Microsoft Windows XP Professional Service Pack 2 (Build 2600)
 *      Using renderer: OpenGL 3.3.0
 *      GeForce GTS 250/PCI/SSE2/3DNOW!: NVIDIA Corporation
 *      OpenGL driver version is 1.2 or better.
 *      GLSL version: 3.3
 */

#include <irrlicht.h>

using namespace irr;



f32 getHeight( const f32 x, const f32 z,
        scene::ISceneCollisionManager* colliman,
        scene::IMeshSceneNode* testNode )
{
    core::vector3df collisionPoint = core::vector3df();
    core::triangle3df collisionTriangle = core::triangle3df();
    const scene::ISceneNode* collisionNode;

    // cast a ray from above the bbox of our testNode straight down
    // and return the height of the mesh at the given xz coordinates
    // or 1000.0f if no collision point was found.
    if ( colliman->getCollisionPoint(
            core::line3df(
                    x, testNode->getTransformedBoundingBox().MaxEdge.Y+1.0f, z,
                    x, testNode->getTransformedBoundingBox().MinEdge.Y-1.0f, z
            ),
            testNode->getTriangleSelector(),
            collisionPoint, collisionTriangle, collisionNode ) )
    {
        return collisionPoint.Y;
    }
    else
    {
        return 1000.0f;
    }
}


// from the 04.Movement example
class MyEventReceiver : public IEventReceiver
{
public:
    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
        // Remember whether each key is down or up
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

        return false;
    }

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(EKEY_CODE keyCode) const
    {
        return KeyIsDown[keyCode];
    }

    MyEventReceiver()
    {
        for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
            KeyIsDown[i] = false;
    }

private:
    // We use this array to store the current state of each key
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
};



int main()
{
    scene::IMesh* testMesh;
    scene::IMeshSceneNode* testNode;
    scene::ITriangleSelector* selector;
    core::matrix4 matrix = core::matrix4();
    scene::ICameraSceneNode* camera;
    core::vector3df cameraOffset = core::vector3df( 0.0f, 2.0f, 0.0f );

    MyEventReceiver receiver;
    IrrlichtDevice* device = createDevice( video::EDT_OPENGL,
            core::dimension2du( 800, 600 ), 32, false, true, false,
            &receiver );
    if ( device == 0 ) exit( 1 );
    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr  = device->getSceneManager();
    scene::IMeshManipulator* meshManipulator = smgr->getMeshManipulator();
    scene::ISceneCollisionManager* colliman = smgr->getSceneCollisionManager();

    // load, scale and place mesh
    driver->setTransform( video::ETS_WORLD, core::matrix4() );
    testMesh = meshManipulator->createMeshCopy(
            smgr->getMesh( "just_a_mesh.3ds" ) );
    matrix.setScale( core::vector3df(
            200.0f + 0.001f, 200.0f, 200.0f + 0.001f ) );
    matrix.setTranslation(
            core::vector3df(
                    99 * 400.0f + 200.0f,
                    0.0f,
                    99 * 400.0f + 200.0f
            )
    );
    meshManipulator->transform( testMesh, matrix );
    meshManipulator->recalculateNormals( testMesh, true );

    // setHardwareMappingHint
    for ( register u32 i = 0; i < testMesh->getMeshBufferCount(); ++i )
    {
        scene::IMeshBuffer* buffer = testMesh->getMeshBuffer( i );
        buffer->setHardwareMappingHint( scene::EHM_STATIC );
        buffer->setDirty();
        buffer->recalculateBoundingBox();
    }

    // create node from mesh
    testNode = smgr->addMeshSceneNode( testMesh );
    testMesh->drop();

    // set material
    testNode->setMaterialTexture(
            0, driver->getTexture( "just_a_texture.jpg" ) );
    testNode->setMaterialType( video::EMT_SOLID );
    testNode->setMaterialFlag( video::EMF_LIGHTING, false );
    testNode->setVisible( true );

    // add node to collision detection
    selector = smgr->createOctreeTriangleSelector(
            testNode->getMesh(), testNode, 900 );
    testNode->setTriangleSelector( selector );
    selector->drop();

    // add a camera
    camera = smgr->addCameraSceneNodeFPS( 0, 360.0f, 0.01f );
    camera->setPosition( core::vector3df(39936.0f, 0.0f, 39755.0f) );
    camera->setTarget( camera->getPosition() + core::vector3df( 1.0f, 0.0f, 1.0f ) );
    camera->updateAbsolutePosition();
    camera->setFarValue( 300.0f );
    camera->setNearValue( 0.1f );
    camera->setFOV( 1.25f );
    camera->setAspectRatio( 4.0f / 3.0f );
    camera->setInputReceiverEnabled( true );
    smgr->setActiveCamera( camera );
    device->getCursorControl()->setVisible( false );

    while( device->run() )
    {
        if ( !device->isWindowActive() ) device->yield();
        if ( receiver.IsKeyDown( irr::KEY_ESCAPE ) ) device->closeDevice();

        core::vector3df pos = camera->getPosition() - cameraOffset;
        pos.Y = getHeight( pos.X, pos.Z, colliman, testNode );
        if ( pos.Y > 999.0f ) printf( "gap at x=%f, z=%f!\n", pos.X, pos.Z );
        camera->setPosition( pos + cameraOffset );

        driver->beginScene( true, true );
        smgr->drawAll();
        driver->endScene();
    }

    device->drop();
    return 0;
}

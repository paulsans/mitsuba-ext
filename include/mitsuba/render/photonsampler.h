#include <mitsuba/core/fwd.h>
#include <mitsuba/core/spectrum.h>
#include <iostream>
#include <vector>
// using namespace std;

using namespace mitsuba;

class PhotonRay {
private:
    Point m_position;
    Vector m_direction;
    Spectrum m_spectrum;
    bool m_isValid;
public:
    PhotonRay(Point position, Vector direction, Spectrum spectrum, bool isValid=true) 
        : m_position(position), m_direction(direction), m_spectrum(spectrum), m_isValid(isValid) {}

    PhotonRay(Ray &ray, Spectrum power, bool isValid=true) : PhotonRay(ray.o, ray.d, power, isValid) {}
    PhotonRay(Point position, bool isValid=true) : m_position(position), m_isValid(isValid) {}
    PhotonRay() : m_isValid(false) {}

    Point getPosition() const { return m_position; }
    Vector getDirection() const { return m_direction; }
    Spectrum getSpectrum() const { return m_spectrum; }
    bool isValid() const {return m_isValid; }

    bool operator==(const PhotonRay &other) const {
        return this->m_position == other.getPosition() && this->m_direction == other.getDirection() && this->m_spectrum == other.getSpectrum();
    }
};



class PhotonSampler {
private:
    ref<Scene> m_scene;

public:
    PhotonSampler(Scene * scene) {
        assert(scene != NULL);
        m_scene = scene;
    }

    /* return a numPhotons * depth sized vector of photon rays */
    std::vector<PhotonRay> samplePhotons(int numPhotons, int depth, bool enforceIntersection) {
        ref<Sampler> sampler = static_cast<Sampler *> (PluginManager::getInstance()->
            createObject(MTS_CLASS(Sampler), Properties("halton")));
        std::vector<PhotonRay> photonRays;

        const Emitter *emitter = NULL;

        Spectrum power;

        ref<Sensor> sensor;

        m_scene->getSensor();
        sensor    = m_scene->getSensor();

        PositionSamplingRecord pRec(sensor->getShutterOpen() + 0.5f * sensor->getShutterOpenTime());
        Ray ray;

        Intersection its;

        for (int i = 0; i < numPhotons; i++) {
            sampler->setSampleIndex(i);

            power = m_scene->sampleEmitterRay(ray, emitter, sampler->next2D(), sampler->next2D(), pRec.time);

            photonRays.push_back(PhotonRay(ray, power));

            PhotonRay photonRay = PhotonRay();

            bool validPhoton = false;
            if (m_scene->rayIntersect(ray, its)) {
                if (its.isValid()) {
                    photonRay = PhotonRay(its.p, true);    
                    validPhoton = true;
                }
            }

            photonRays.push_back(photonRay);
        }

        return photonRays;
    }

    virtual ~PhotonSampler() = default;
};
